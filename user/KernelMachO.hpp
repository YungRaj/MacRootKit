#ifndef __KERNEL_MACHO_HPP_
#define __KERNEL_MACHO_HPP_

#include "MachO.hpp"

namespace xnu
{
	class Kernel;

	class KernelMachO : public MachO 
	{
		public:
			KernelMachO() { }

			KernelMachO(uintptr_t address);
			KernelMachO(uintptr_t address, off_t slide);

			KernelMachO(const char *path, off_t slide);
			KernelMachO(const char *path);

			~KernelMachO();

			virtual void parseLinkedit();

			virtual bool parseLoadCommands();

			virtual void parseMachO();

		private:
			xnu::Kernel *kernel;

		protected:
			uint8_t *linkedit;

			mach_vm_address_t linkedit_off;
			
			size_t linkedit_size;
	};

	class KernelCacheMachO : public KernelMachO
	{
		public:
			KernelCacheMachO(mach_vm_address_t kc, uintptr_t address);
			KernelCacheMachO(mach_vm_address_t kc, uintptr_t address, off_t slide);

			virtual bool parseLoadCommands();

		private:
			mach_vm_address_t kernel_cache;
	};

}

#endif