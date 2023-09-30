#include "MacRootKit.hpp"

#include "Kext.hpp"
#include "KextMachO.hpp"

#include "Kernel.hpp"
#include "KernelMachO.hpp"

#include "MachO.hpp"

using namespace xnu;

Kext::Kext(Kernel *kernel, mach_vm_address_t base, char *identifier)
    : kernel(kernel),
      address(base),
      identifier(identifier)
{
    macho = new KextMachO(kernel, identifier, address);

    kmod_info = reinterpret_cast<kmod_info_t*>(macho->getSymbolAddressByName("_kmod_info"));

    size = kmod_info->size;
}

Kext::Kext(Kernel *kernel, void *kext, kmod_info_t *kmod_info)
    : kernel(kernel),
      kext(kext),
      kmod_info(kmod_info),
      address(kmod_info->address),
      size(kmod_info->size),
      identifier(&kmod_info->name[0])
{
	macho = kmod_info->address ? new KextMachO(kernel, identifier, kmod_info) : NULL;
}

Kext::~Kext()
{

}

Kext* Kext::findKextWithIdentifier(Kernel *kernel, char *name)
{
	mrk::MacRootKit *rootkit = kernel->getRootKit();

	return rootkit->getKextByIdentifier(name);
}

Kext* Kext::findKextWithIdentifier_deprecated(Kernel *kernel, char *name)
{
	kmod_info_t **kextKmods;

	mach_vm_address_t _kmod = kernel->getSymbolAddressByName("_kmod");

	kextKmods = reinterpret_cast<kmod_info_t**>(_kmod);

	for(kmod_info_t *kmod = *kextKmods; kmod; kmod = kmod->next)
	{
		if(strcmp(kmod->name, name) == 0)
		{
			Kext *kext;

			void *OSKext;

			typedef void* (*lookupKextWithIdentifier)(const char*);

			void* (*__ZN6OSKext24lookupKextWithIdentifierEPKc)(const char*);

			mach_vm_address_t OSKext_lookupWithIdentifier = kernel->getSymbolAddressByName("__ZN6OSKext24lookupKextWithIdentifierEPKc");

			__ZN6OSKext24lookupKextWithIdentifierEPKc = reinterpret_cast<lookupKextWithIdentifier>(OSKext_lookupWithIdentifier);

			OSKext = __ZN6OSKext24lookupKextWithIdentifierEPKc(name);

			kext = new Kext(kernel, OSKext, kmod);
		}
	}

	return NULL;
}

Kext* Kext::findKextWithId(Kernel *kernel, uint32_t kext_id)
{
	return NULL;
}

void Kext::onKextLoad(void *kext, kmod_info_t *kmod_info)
{
	return;
}
