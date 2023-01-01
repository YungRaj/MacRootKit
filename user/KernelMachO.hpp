#ifndef __KERNEL_MACHO_HPP_
#define __KERNEL_MACHO_HPP_

#include "Kernel.hpp"
#include "MachO.hpp"

class MachO;

namespace xnu
{
	class Kernel;

	class KernelMachO : public MachO 
	{
		public:
			KernelMachO(const char *path);

			~KernelMachO();

			virtual void parseLinkedit();

			virtual bool parseLoadCommands();

			virtual void parseMachO();

		private:
			xnu::Kernel *kernel;

			uint8_t *linkedit;

			mach_vm_address_t linkedit_off;
			
			size_t linkedit_size;
	};

}

#endif