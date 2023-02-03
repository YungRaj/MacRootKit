#include "Reg_x86_64.hpp"

namespace Arch
{
	namespace x86_64
	{
		uintptr_t getCR0()
		{
			uintptr_t cr0;

		#if __x86_64__
			__asm__ volatile("mov %%cr0, %0" : "=r" (cr0));
		#endif

			return cr0;
		}

		void setCR0(uintptr_t value)
		{
		
		#if __x86_64__
			__asm__ volatile("mov %0, %%cr0" : : "r" (value));
		#endif
		
		}

		uintptr_t getEFER()
		{
			uintptr_t efer;

		#if __x86_64__
			__asm__ volatile("rdmsr" : : "A" (efer), "c" (MSR_IA32_EFER));
		#endif

			return efer;
		}

		void setEFER(uintptr_t value)
		{
			uintptr_t efer;
			
		#if __x86_64__
			__asm__ volatile("rdmsr" : : "A" (efer), "c" (MSR_IA32_EFER));
			__asm__ volatile("wrmsr" : : "a" (value), "c" (MSR_IA32_EFER));
		#endif

		}
	}
}