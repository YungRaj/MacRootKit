#include "MacRootKit.hpp"

using namespace Arch;
using namespace xnu;

namespace mrk
{

MacRootKit::MacRootKit(Kernel *kernel)
{
	this->kernel = kernel;
	this->kernel->setRootKit(this);

	this->kextKmods = reinterpret_cast<kmod_info_t**>(kernel->getSymbolAddressByName("_kmod"));

	this->platformArchitecture = Arch::getCurrentArchitecture();;

	this->registerCallbacks();

	this->kernelPatcher = new KernelPatcher(this->kernel);

	this->architecture = Arch::initArchitecture();
}

MacRootKit::~MacRootKit()
{

}

void MacRootKit::registerCallbacks()
{
	this->registerEntitlementCallback((void*) this, [] (void *user, task_t task, const char *entitlement, void *original)
	{
		static_cast<MacRootKit*>(user)->onEntitlementRequest(task, entitlement, original);
	});

	this->registerBinaryLoadCallback((void*) this, [] (void *user, task_t task, const char *path, size_t len)
	{
		static_cast<MacRootKit*>(user)->onProcLoad(task, path, len);
	});

	this->registerKextLoadCallback((void*) this, [] (void *user, void *kext, kmod_info_t *kmod)
	{
		static_cast<MacRootKit*>(user)->onKextLoad(kext, kmod);
	});
}

void MacRootKit::registerEntitlementCallback(void *user, entitlement_callback_t callback)
{
	StoredPair<entitlement_callback_t> *pair = StoredPair<entitlement_callback_t>::create(callback, user);

	this->entitlementCallbacks.add(pair);
}

void MacRootKit::registerBinaryLoadCallback(void *user, binaryload_callback_t callback)
{
	StoredPair<binaryload_callback_t> *pair = StoredPair<binaryload_callback_t>::create(callback, user);

	this->binaryLoadCallbacks.add(pair);
}

void MacRootKit::registerKextLoadCallback(void *user, kextload_callback_t callback)
{
	StoredPair<kextload_callback_t> *pair = StoredPair<kextload_callback_t>::create(callback, user);

	this->kextLoadCallbacks.add(pair);
}

Kext* MacRootKit::getKextByIdentifier(char *name)
{
	std::Array<Kext*> *kexts = this->getKexts();

	for(int i = 0; i < kexts->getSize(); i++)
	{
		Kext *kext = kexts->get(i);

		if(strcmp(kext->getName(), name) == 0)
		{
			return kext;
		}
	}

	return NULL;
}

Kext* MacRootKit::getKextByAddress(mach_vm_address_t address)
{
	std::Array<Kext*> *kexts = this->getKexts();

	for(int i = 0; i < kexts->getSize(); i++)
	{
		Kext *kext = kexts->get(i);

		if(kext->getAddress() == address)
		{
			return kext;
		}
	}

	return NULL;
}

void MacRootKit::onEntitlementRequest(task_t task, const char *entitlement, void *original)
{

}

void MacRootKit::onProcLoad(task_t task, const char *path, size_t len)
{

}

void MacRootKit::onKextLoad(void *loaded_kext, kmod_info_t *kmod_info)
{
	Kext *kext;

	if(loaded_kext)
	{
		kext = new Kext(this->getKernel(), loaded_kext, kmod_info);
	} else
	{
		kext = new Kext(this->getKernel(), kmod_info->address, reinterpret_cast<char*>(&kmod_info->name));
	}

	kexts.add(kext);
}

kmod_info_t* MacRootKit::findKmodInfo(const char *kextname)
{
	kmod_info_t *kmod;

	if(!kextKmods)
		return NULL;

	for(kmod = *kextKmods; kmod; kmod = kmod->next)
	{
		if(strcmp(kmod->name, kextname) == 0)
		{
			return kmod;
		}
	}

	return NULL;
}

void* MacRootKit::findOSKextByIdentifier(const char *kextidentifier)
{
	void* (*lookupKextWithIdentifier)(const char*);

	typedef void* (*__ZN6OSKext24lookupKextWithIdentifierEPKc)(const char*);

	lookupKextWithIdentifier = reinterpret_cast<__ZN6OSKext24lookupKextWithIdentifierEPKc>(this->kernel->getSymbolAddressByName("__ZN6OSKext24lookupKextWithIdentifierEPKc"));

	void *OSKext = lookupKextWithIdentifier(kextidentifier);

	return OSKext;
}

}