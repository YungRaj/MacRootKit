#ifndef __KEXT_HPP_
#define __KEXT_HPP_

#include <stddef.h>
#include <stdint.h>

#include <mach/kmod.h>
#include <mach/mach_types.h>

#include "KextMachO.hpp"

namespace xnu {}

namespace xnu
{
	class Kernel;

	class Kext
	{
		public:
			Kext(xnu::Kernel *kernel, mach_vm_address_t base, char *identifier);

			Kext(xnu::Kernel *kernel, void *kext, kmod_info_t *kmod_info);

			~Kext();

			static xnu::Kext* findKextWithIdentifier(xnu::Kernel *kernel, char *name);
			static xnu::Kext* findKextWithIdentifier_deprecated(xnu::Kernel *kernel, char *name);

			static xnu::Kext* findKextWithId(xnu::Kernel *kernel, uint32_t kext_id);

			static void onKextLoad(void *kext, kmod_info_t *kmod_info);

			char* getName() { return identifier; }

			mach_vm_address_t getBase() { return address; }

			mach_vm_address_t getAddress() { return address; }

			size_t getSize() { return size; }

			void* getOSKext() { return kext; }

			kmod_info_t* getKmodInfo() { return kmod_info; }

			kmod_start_func_t* getKmodStart() { return kmod_info->start; }
			kmod_stop_func_t*  getKmodStop() { return kmod_info->stop; }

			Symbol* getSymbolByName(char *symname) { return macho->getSymbolByName(symname); }
			Symbol* getSymbolByAddress(mach_vm_address_t address) { return macho->getSymbolByAddress(address); }

			mach_vm_address_t getSymbolAddressByName(char *symbolname) { return macho->getSymbolAddressByName(symbolname); }

		private:
			xnu::Kernel *kernel;

			xnu::KextMachO *macho;

			kmod_info_t *kmod_info;

			void *kext;

			mach_vm_address_t address;

			size_t size;

			char *identifier;
	};

};

#endif