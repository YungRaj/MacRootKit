#ifndef __KEXT_MACHO_HPP_
#define __KEXT_MACHO_HPP_

#include <mach/mach_types.h>
#include <mach/kmod.h>

#include "MachO.hpp"

#include "Array.hpp"
#include "SymbolTable.hpp"

namespace xnu
{
	class Kernel;
	class Kext;
}

namespace xnu
{
	class KextMachO : public MachO
	{
		public:
			KextMachO(xnu::Kernel *kernel, char *name, mach_vm_address_t base);
			KextMachO(xnu::Kernel *kernel, char *name, kmod_info_t *kmod_info);

			~KextMachO();

			xnu::Kernel* getKernel() { return kernel; }

			char* getKextName() { return name; }

			mach_vm_address_t getAddress() { return address; }

			virtual size_t getSize() { return kmod_info->size > 0 ? kmod_info->size : MachO::getSize(); }

			kmod_start_func_t* getKmodStart() { return kmod_info->start; }
			kmod_stop_func_t* getKmodStop() { return kmod_info->stop; }

			void setKernelCollection(mach_vm_address_t kc) { this->kernel_collection = kc; }

			virtual void parseLinkedit();

			virtual bool parseLoadCommands();

			virtual void parseHeader();

			virtual void parseMachO();

		private:
			xnu::Kernel *kernel;

			mach_vm_address_t address;

			char *name;

			off_t base_offset;

			mach_vm_address_t kernel_cache;
			mach_vm_address_t kernel_collection;

			kmod_info_t *kmod_info;

			uint8_t *linkedit;

			off_t linkedit_off;

			size_t linkedit_size;
	};
}

#endif