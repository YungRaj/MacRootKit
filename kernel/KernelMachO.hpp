#ifndef __KERNEL_MACHO_HPP_
#define __KERNEL_MACHO_HPP_

#include "Kernel.hpp"
#include "MachO.hpp"

class MachO;

namespace xnu
{
	class Kernel;
	class Kext;

	class KernelMachO : public MachO 
	{
		public:
			KernelMachO(xnu::Kernel *kernel);

			~KernelMachO();

			mach_vm_address_t getKernelCache() { return kernel_cache; }

			mach_vm_address_t getKernelCollection() { return kernel_collection; }

			void setKernelCache(mach_vm_address_t kc) { this->kernel_cache = kc; }

			void setKernelCollection(mach_vm_address_t kc) { this->kernel_collection = kc; }

			static xnu::Kext* kextLoadedAt(xnu::Kernel *kernel, mach_vm_address_t address);
			static xnu::Kext* kextWithIdentifier(xnu::Kernel *kernel, char *kext);

			virtual void parseLinkedit();

			virtual bool parseLoadCommands();

			virtual void parseMachO();

		protected:
			xnu::Kernel *kernel;

			mach_vm_address_t kernel_cache;

			mach_vm_address_t kernel_collection;

			uint8_t *linkedit;

			mach_vm_address_t linkedit_off;
			
			size_t linkedit_size;
	};

}

#endif