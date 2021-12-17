#include "Reg_x86_64.hpp"

namespace Arch
{
	namespace x86_64
	{
		uintptr_t getCR0()
		{
			uintptr_t cr0;

			__asm__ volatile("mov %%cr0, %0" : "=r" (cr0));

			return cr0;
		}

		void setCR0(uintptr_t value)
		{
			__asm__ volatile("mov %0, %%cr0" : : "r" (value));
		}

		uintptr_t getEFER()
		{
			uintptr_t efer;

			__asm__ volatile("rdmsr" : : "A" (efer), "c" (MSR_IA32_EFER));

			return efer;
		}

		void setEFER(uintptr_t value)
		{
			uintptr_t efer;
			
			__asm__ volatile("rdmsr" : : "A" (efer), "c" (MSR_IA32_EFER));
			__asm__ volatile("wdmsr" : : "a" (value), "c" (MSR_IA32_EFER));
		}
	}
}