#ifndef __HYPERVISOR_HPP_
#define __HYPERVISOR_HPP_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <sys/mman.h>

#include <Hypervisor/Hypervisor.h>

#include "Fuzzer.hpp"

#include "virt.h"

namespace Virtualization
{
	/* Valid Syndrome Register EC field values */
	enum arm_exception_class
	{
		EC_UNCATEGORIZED          = 0x00,
		EC_WFX_TRAP               = 0x01,
		EC_CP15RTTRAP             = 0x03,
		EC_CP15RRTTRAP            = 0x04,
		EC_CP14RTTRAP             = 0x05,
		EC_CP14DTTRAP             = 0x06,
		EC_ADVSIMDFPACCESSTRAP    = 0x07,
		EC_FPIDTRAP               = 0x08,
		EC_PACTRAP                = 0x09,
		EC_BXJTRAP                = 0x0a,
		EC_CP14RRTTRAP            = 0x0c,
		EC_BTITRAP                = 0x0d,
		EC_ILLEGALSTATE           = 0x0e,
		EC_AA32_SVC               = 0x11,
		EC_AA32_HVC               = 0x12,
		EC_AA32_SMC               = 0x13,
		EC_AA64_SVC               = 0x15,
		EC_AA64_HVC               = 0x16,
		EC_AA64_SMC               = 0x17,
		EC_SYSTEMREGISTERTRAP     = 0x18,
		EC_SVEACCESSTRAP          = 0x19,
		EC_SMETRAP                = 0x1d,
		EC_INSNABORT              = 0x20,
		EC_INSNABORT_SAME_EL      = 0x21,
		EC_PCALIGNMENT            = 0x22,
		EC_DATAABORT              = 0x24,
		EC_DATAABORT_SAME_EL      = 0x25,
		EC_SPALIGNMENT            = 0x26,
		EC_AA32_FPTRAP            = 0x28,
		EC_AA64_FPTRAP            = 0x2c,
		EC_SERROR                 = 0x2f,
		EC_BREAKPOINT             = 0x30,
		EC_BREAKPOINT_SAME_EL     = 0x31,
		EC_SOFTWARESTEP           = 0x32,
		EC_SOFTWARESTEP_SAME_EL   = 0x33,
		EC_WATCHPOINT             = 0x34,
		EC_WATCHPOINT_SAME_EL     = 0x35,
		EC_AA32_BKPT              = 0x38,
		EC_VECTORCATCH            = 0x3a,
		EC_AA64_BKPT              = 0x3c,
		EC_AA64_APPLE             = 0x3f,
	};

	// ARM64 reset tramp
	// I don't know why the CPU resets at EL0, so this is a trampoline
	// that takes you to EL1.
	// UPDATE: See CPSR below
	const uint8_t sArm64ResetVector[] = {
	    0x01, 0x00, 0x00, 0xD4, // svc #0
	    // If BRK is caught by host (configured)
	    // Something happened badly
	    0x00, 0x00, 0x20, 0xD4, // brk #0
	};

	const uint8_t sArm64ResetTrampoline[] = {
	    0x00, 0x00, 0xB0, 0xD2, // mov x0, #0x80000000
	    0x00, 0x00, 0x1F, 0xD6, // br  x0
	    // If BRK is caught by host (configured)
	    // Something happened badly
	    0x00, 0x00, 0x20, 0xD4, // brk #0
	};

	#define HYP_ASSERT_SUCCESS(ret) assert((hv_return_t) (ret) == HV_SUCCESS)

	// Overview of this memory layout:
	// Main memory starts at 0x80000000
	// Reset VBAR_EL1 at 0xF0000000 - 0xF0010000;
	// Reset trampoline code at 0xF0000800
	const uint64_t gAdrResetTrampoline = 0xF0000000;
	const uint64_t gResetTrampolineMemorySize = 0x10000;

	const uint64_t gMainMemory = 0x80000000;
	const uint64_t gMainMemSize = 0x1000000;

	class Hypervisor
	{
		public:
			explicit Hypervisor(Fuzzer::Harness *harness, mach_vm_address_t base, size_t size, mach_vm_address_t entryPoint);

			Fuzzer::Harness* getHarness() { return harness; }

			mach_vm_address_t getBase() { return base; }

			size_t getSize() { return size; }

			mach_vm_address_t getEntryPoint() { return entryPoint; }

			hv_vcpu_t getVirtualCpu() { return vcpu; }

			hv_vcpu_exit_t* getVirtualCpuExit() {  return vcpu_exit; }

			int sysregRead(uint32_t reg, uint32_t rt);
			int sysregWrite(uint32_t reg, uint64_t val);

			void* getPhysicalMainMemory() { return mainMemory; }

			int prepareSystemMemory();

			void configure();

			void start();

			void destroy();

		private:
			Fuzzer::Harness *harness;

			HvfArm64State state;

			mach_vm_address_t base;

			size_t size;

			mach_vm_address_t entryPoint;

			hv_vcpu_t vcpu;

			hv_vcpu_exit_t *vcpu_exit;

			void* resetTrampolineMemory;
			void* mainMemory;

	};
}

#endif