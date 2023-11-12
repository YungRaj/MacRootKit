#include "Hypervisor.hpp"

#include "KernelMachO.hpp"

static inline int64_t get_clock(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000000000LL + ts.tv_nsec;
}


Virtualization::Hypervisor::Hypervisor(Fuzzer::Harness *harness, mach_vm_address_t base, size_t size, mach_vm_address_t entryPoint)
: harness(harness),
  base(base),
  size(size),
  entryPoint(entryPoint)
{
	int ret;

	// Create the VM
	HYP_ASSERT_SUCCESS(hv_vm_create(NULL));

	ret = prepareSystemMemory();

	if(ret != 0)
	{
		printf("Failed to prepare Hypervisor's System Memory!\n");

		exit(-1);
	}

	configure();

	start();
}

int Virtualization::Hypervisor::sysregRead(uint32_t reg, uint32_t rt)
{
	HvfArm64State *env = &state;

    uint64_t val = 0;

    switch (reg)
    {
    case SYSREG_CNTPCT_EL0:
    	uint64_t gt_cntfrq_hz;

    	 asm volatile("mrs %0, cntfrq_el0" : "=r"(gt_cntfrq_hz));

    	#define NANOSECONDS_PER_SECOND 1000000000LL

        val = get_clock() / (NANOSECONDS_PER_SECOND > gt_cntfrq_hz ?
      NANOSECONDS_PER_SECOND / gt_cntfrq_hz : 1);
        break;
    case SYSREG_PMCR_EL0:
        val = env->cp15.c9_pmcr;
        break;
    case SYSREG_PMCCNTR_EL0:
        // pmu_op_start(env);
        val = env->cp15.c15_ccnt;
        // pmu_op_finish(env);
        break;
    case SYSREG_PMCNTENCLR_EL0:
        val = env->cp15.c9_pmcnten;
        break;
    case SYSREG_PMOVSCLR_EL0:
        val = env->cp15.c9_pmovsr;
        break;
    case SYSREG_PMSELR_EL0:
        val = env->cp15.c9_pmselr;
        break;
    case SYSREG_PMINTENCLR_EL1:
        val = env->cp15.c9_pminten;
        break;
    case SYSREG_PMCCFILTR_EL0:
        val = env->cp15.pmccfiltr_el0;
        break;
    case SYSREG_PMCNTENSET_EL0:
        val = env->cp15.c9_pmcnten;
        break;
    case SYSREG_PMUSERENR_EL0:
        val = env->cp15.c9_pmuserenr;
        break;
    case SYSREG_PMCEID0_EL0:
    case SYSREG_PMCEID1_EL0:
        /* We can't really count anything yet, declare all events invalid */
        val = 0;
        break;
    case SYSREG_OSLSR_EL1:
        val = env->cp15.oslsr_el1;
        break;
    case SYSREG_OSDLR_EL1:
        /* Dummy register */
        break;
    default:
        break;
    }

    HYP_ASSERT_SUCCESS(hv_vcpu_set_reg(vcpu, (hv_reg_t) (HV_REG_X0 + rt), val));

    return 0;
}

int Virtualization::Hypervisor::sysregWrite(uint32_t reg, uint64_t val)
{
	HvfArm64State *env = &state;

    switch (reg)
    {
    case SYSREG_PMCCNTR_EL0:
        env->cp15.c15_ccnt = val;
        break;
    case SYSREG_PMCR_EL0:
        if (val & PMCRC) {
            /* The counter has been reset */
            env->cp15.c15_ccnt = 0;
        }

        if (val & PMCRP) {
            unsigned int i;
            for (i = 0; i < pmu_num_counters(env); i++) {
                env->cp15.c14_pmevcntr[i] = 0;
            }
        }

        env->cp15.c9_pmcr &= ~PMCR_WRITABLE_MASK;
        env->cp15.c9_pmcr |= (val & PMCR_WRITABLE_MASK);

        break;
    case SYSREG_PMUSERENR_EL0:
        env->cp15.c9_pmuserenr = val & 0xf;
        break;
    case SYSREG_PMCNTENSET_EL0:
        env->cp15.c9_pmcnten |= (val & pmu_counter_mask(env));
        break;
    case SYSREG_PMCNTENCLR_EL0:
        env->cp15.c9_pmcnten &= ~(val & pmu_counter_mask(env));
        break;
    case SYSREG_PMINTENCLR_EL1:
        env->cp15.c9_pminten |= val;
        break;
    case SYSREG_PMOVSCLR_EL0:
        env->cp15.c9_pmovsr &= ~val;
        break;
    case SYSREG_PMSWINC_EL0:
        break;
    case SYSREG_PMSELR_EL0:
        env->cp15.c9_pmselr = val & 0x1f;
        break;
    case SYSREG_PMCCFILTR_EL0:
        env->cp15.pmccfiltr_el0 = val & PMCCFILTR_EL0;
        break;
    case SYSREG_OSLAR_EL1:
        env->cp15.oslsr_el1 = val & 1;
        break;
    case SYSREG_OSDLR_EL1:
        /* Dummy register */
        break;
    default:
        break;
    }

    return 0;
}

int Virtualization::Hypervisor::prepareSystemMemory()
{
	// Reset trampoline
	// Well, dear Apple, why you reset the CPU at EL0
	posix_memalign(&g_pResetTrampolineMemory, 0x10000, g_szResetTrampolineMemory);

	if (g_pResetTrampolineMemory == NULL)
	{
		printf("Failed to posix_memalign() g_pMainMemory!\n");

		return -ENOMEM;
	}

	memset(g_pResetTrampolineMemory, 0, g_szResetTrampolineMemory);

	for (uint64_t offset = 0; offset < 0x780; offset += 0x80)
	{
		memcpy((void*) ((uint64_t) g_pResetTrampolineMemory + offset), s_cArm64ResetTramp, sizeof(s_cArm64ResetTramp));
	}

	memcpy((void*) ((uint64_t) g_pResetTrampolineMemory + 0x800), s_cArm64ResetVector, sizeof(s_cArm64ResetVector));

	// Map the RAM into the VM
	HYP_ASSERT_SUCCESS(hv_vm_map(g_pResetTrampolineMemory, g_kAdrResetTrampoline, g_szResetTrampolineMemory, HV_MEMORY_READ | HV_MEMORY_EXEC));

	// Main memory.
	posix_memalign(&g_pMainMemory, 0x1000, g_szMainMemSize);

	if (g_pMainMemory == NULL) 
	{
		printf("Failed to posix_memalign() g_pMainMemory!\n");

		return -ENOMEM;
	}

	// Copy our code into the VM's RAM
	memset(g_pMainMemory, 0, g_szMainMemSize);
	memcpy(g_pMainMemory, (void*) base, size);

	// Map the RAM into the VM
	HYP_ASSERT_SUCCESS(hv_vm_map(g_pMainMemory, g_kAdrMainMemory, g_szMainMemSize, HV_MEMORY_READ | HV_MEMORY_WRITE | HV_MEMORY_EXEC));

	return 0;
}

void Virtualization::Hypervisor::configure()
{
	mach_vm_address_t __start = (entryPoint - base) + g_kAdrMainMemory;

	printf("__start physical address = 0x%llx\n", __start);

	// Add a virtual CPU to our VM
	HYP_ASSERT_SUCCESS(hv_vcpu_create(&vcpu, &vcpu_exit, NULL));

	// Configure initial VBAR_EL1 to the trampoline
	HYP_ASSERT_SUCCESS(hv_vcpu_set_sys_reg(vcpu, HV_SYS_REG_VBAR_EL1, g_kAdrResetTrampoline));

	#if USE_EL0_TRAMPOILNE
	// Set the CPU's PC to execute from the trampoline
	HYP_ASSERT_SUCCESS(hv_vcpu_set_reg(vcpu, HV_REG_PC, g_kAdrResetTrampoline + 0x800));
	#else
	// Or explicitly set CPSR
	HYP_ASSERT_SUCCESS(hv_vcpu_set_reg(vcpu, HV_REG_CPSR, 0x3c4));
	HYP_ASSERT_SUCCESS(hv_vcpu_set_reg(vcpu, HV_REG_PC, __start));
	#endif

	// Configure misc
	HYP_ASSERT_SUCCESS(hv_vcpu_set_sys_reg(vcpu, HV_SYS_REG_SP_EL0, g_kAdrMainMemory + size + 0x4000));
	HYP_ASSERT_SUCCESS(hv_vcpu_set_sys_reg(vcpu, HV_SYS_REG_SP_EL1, g_kAdrMainMemory + size + 0x8000));

	// Trap debug access (BRK)
	HYP_ASSERT_SUCCESS(hv_vcpu_set_trap_debug_exceptions(vcpu, true));
}

void Virtualization::Hypervisor::start()
{
	// start the VM
	while (true)
	{
		// Run the VM until a VM exit
		HYP_ASSERT_SUCCESS(hv_vcpu_run(vcpu));
		// Check why we exited the VM
		if (vcpu_exit->reason == HV_EXIT_REASON_EXCEPTION)
		{
			// Check if this is an HVC call
			// https://developer.arm.com/docs/ddi0595/e/aarch64-system-registers/esr_el2
			uint64_t syndrome = vcpu_exit->exception.syndrome;
			uint8_t ec = (syndrome >> 26) & 0x3f;
			// check Exception Class

			if (ec == EC_AA32_HVC)
			{
				// Exception Class 0x16 is
				// "HVC instruction execution in AArch64 state, when HVC is not disabled."
				uint64_t x0;
				HYP_ASSERT_SUCCESS(hv_vcpu_get_reg(vcpu, HV_REG_X0, &x0));
				printf("VM made an HVC call! x0 register holds 0x%llx\n", x0);
				break;
			} else if (ec == EC_AA64_SMC)
			{
				// Exception Class 0x17 is
				// "SMC instruction execution in AArch64 state, when SMC is not disabled."

				// Yes despite M1 doesn't have EL3, it is capable to trap it too. :)
				uint64_t x0;
				HYP_ASSERT_SUCCESS(hv_vcpu_get_reg(vcpu, HV_REG_X0, &x0));
				printf("VM made an SMC call! x0 register holds 0x%llx\n", x0);
				printf("Return to get on next instruction.\n");

				// ARM spec says trapped SMC have different return path, so it is required
				// to increment elr_el2 by 4 (one instruction.)
				uint64_t pc;
				HYP_ASSERT_SUCCESS(hv_vcpu_get_reg(vcpu, HV_REG_PC, &pc));
				pc += 4;
				HYP_ASSERT_SUCCESS(hv_vcpu_set_reg(vcpu, HV_REG_PC, pc));

			} else if(ec == EC_SYSTEMREGISTERTRAP)
			{
				uint64_t pc;

				bool isread = (syndrome >> 0) & 1;
				uint32_t rt = (syndrome >> 5) & 0x1f;
				uint32_t reg = syndrome & SYSREG_MASK;
				
				uint64_t val;

				int ret = 0;

				if (isread)
				{
					ret = sysregRead(reg, rt);
				} else
				{
					HYP_ASSERT_SUCCESS(hv_vcpu_get_reg(vcpu, (hv_reg_t) rt, &val));

					ret = sysregWrite(reg, val);
				}

				HYP_ASSERT_SUCCESS(hv_vcpu_get_reg(vcpu, HV_REG_PC, &pc));
				pc += 4;
				HYP_ASSERT_SUCCESS(hv_vcpu_set_reg(vcpu, HV_REG_PC, pc));

			} else if (ec == EC_AA64_BKPT)
			{
				// Exception Class 0x3C is BRK in AArch64 state
				uint64_t x0;

				HYP_ASSERT_SUCCESS(hv_vcpu_get_reg(vcpu, HV_REG_X0, &x0));

				printf("VM made an BRK call!\n");
				printf("Reg dump:\n");

				for (uint32_t reg = HV_REG_X0; reg < HV_REG_X5; reg++)
				{
					uint64_t s;
					HYP_ASSERT_SUCCESS(hv_vcpu_get_reg(vcpu, (hv_reg_t) reg, &s));
					printf("X%d: 0x%llx\n", reg, s);
				}

				break;
			} else
			{
				uint64_t pc;

				HYP_ASSERT_SUCCESS(hv_vcpu_get_reg(vcpu, HV_REG_PC, &pc));

				fprintf(stderr, "Unexpected VM exception: 0x%llx, EC 0x%x, VirtAddr 0x%llx, IPA 0x%llx Reason 0x%x PC 0x%llx\n",
						syndrome,
						ec,
						vcpu_exit->exception.virtual_address,
						vcpu_exit->exception.physical_address,
						vcpu_exit->reason,
						pc
				);
				
				break;
			}
		} else
		{
			fprintf(stderr, "Unexpected VM exit reason: %d\n", vcpu_exit->reason);
			
			break;
		}
	}

	destroy();
}

void Virtualization::Hypervisor::destroy()
{
	// Tear down the VM
    hv_vcpu_destroy(vcpu);
    hv_vm_destroy();

    // Free memory
    free(g_pResetTrampolineMemory);
    free(g_pMainMemory);
}