#ifndef __KEXT_HPP_
#define __KEXT_HPP_

#include <mach/kmod_info.h>

class Kernel;

class Kext
{
	public:
		Kext(Kernel *kernel, void *kext, kmod_info_t *kmod_info);

		~Kext();

		static Kext* findKextWithIdentifier(Kernel *kernel, har *name);
		static Kext* findKextWithIdentifier_deprecated(Kernel *kernel, char *name);

		static Kext* findKextWithId(Kernel *kernel, uint32_t kext_id);

		static void onKextLoad(void *kext, kmod_info_t *kmod_info);

		char* getName() { return identifier; }

		mach_vm_address_t getBase() { return address; }

		mach_vm_address_t getAddress() { return address; }

		size_t getSize() { return size; }

		void* getOSKext() { return kext; }

		kmod_info_t getKmodInfo() { return kmod_info; }

		kmod_start_func_t* getKmodStart() { return kmod_info->start; }
		kmod_stop_func_t*  getKmodStop() { return kmod_info->stop; }

	private:
		Kernel *kernel;

		KextMachO *macho;

		kmod_info_t *kmod_info;

		void *kext;

		mach_vm_address_t address;

		size_t size;

		char *identifier;
};

#endif