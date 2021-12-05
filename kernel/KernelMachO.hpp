#ifndef __KERNEL_MACHO_HPP_
#define __KERNEL_MACHO_HPP_

#include "Kernel.hpp"

#include "MachO.hpp"

class Kernel;
class MachO;

class KernelMachO : public MachO 
{
	public:
		KernelMachO(Kernel *kernel);

		~KernelMachO();

		void setKernelCollection(mach_vm_address_t kc) { this->kernel_collection = kc; }

		static Kext* kextLoadedAt(Kernel *kernel, mach_vm_address_t address);

		virtual void parseLinkedit();

		virtual void parseLoadCommands();

		virtual void parseMachO();

	private:
		Kernel *kernel;

		mach_vm_address_t kernel_collection;

		uint8_t *linkedit;

		mach_vm_address_t linkedit_off;
		
		size_t linkedit_size;
};

#endif