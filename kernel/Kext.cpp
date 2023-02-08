#include "Kext.hpp"
#include "KextMachO.hpp"

#include "Kernel.hpp"
#include "KernelMachO.hpp"

#include "MachO.hpp"

using namespace xnu;

Kext::Kext(Kernel *kernel, mach_vm_address_t base, char *identifier)
{
	this->kernel = kernel;
	this->address = base;
	this->identifier = identifier;
	this->macho = new KextMachO(this->kernel, this->identifier, this->address);
	this->kmod_info = reinterpret_cast<kmod_info_t*>(this->macho->getSymbolAddressByName("_kmod_info"));
	this->size = this->kmod_info->size;
}

Kext::Kext(Kernel *kernel, void *kext, kmod_info_t *kmod_info)
{
	this->kernel = kernel;
	this->kext = kext;
	this->kmod_info = kmod_info;
	this->address = this->kmod_info->address;
	this->size = this->kmod_info->size;
	this->identifier = &this->kmod_info->name[0];

	if(this->kmod_info->address)
		this->macho = new KextMachO(this->kernel, this->identifier, this->kmod_info);
	else
		this->macho = NULL;
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
