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

namespace Virtualization
{
	// ARM64 reset tramp
	// I don't know why the CPU resets at EL0, so this is a trampoline
	// that takes you to EL1.
	// UPDATE: See CPSR below
	const uint8_t s_cArm64ResetVector[] = {
	    0x01, 0x00, 0x00, 0xD4, // svc #0
	    // If BRK is caught by host (configured)
	    // Something happened badly
	    0x00, 0x00, 0x20, 0xD4, // brk #0
	};

	const uint8_t s_cArm64ResetTramp[] = {
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
	const uint64_t g_kAdrResetTrampoline = 0xF0000000;
	const uint64_t g_szResetTrampolineMemory = 0x10000;

	const uint64_t g_kAdrMainMemory = 0x80000000;
	const uint64_t g_szMainMemSize = 0x1000000;

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

			void* getPhysicalMainMemory() { return g_pMainMemory; }

			int prepareSystemMemory();

			void configure();

			void start();

			void destroy();

		private:
			Fuzzer::Harness *harness;

			mach_vm_address_t base;

			size_t size;

			mach_vm_address_t entryPoint;

			hv_vcpu_t vcpu;

    		hv_vcpu_exit_t *vcpu_exit;

    		void* g_pResetTrampolineMemory;
			void* g_pMainMemory;

	};
}

#endif