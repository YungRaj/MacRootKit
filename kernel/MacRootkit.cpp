#include "MacRootKit.hpp"

MacRootKit::MacRootKit(Kernel *kernel)
{
	this->kernel = kernel;
	this->kernel->setRootKit(this);

	this->kextKmods = reinterpret_cast<kmod_info_t**>(kernel->getSymbolAddressByName("_kmod"));

	this->architecture = Arch::getArchitecture();

	this->kernelPatcher = new KernelPatcher(this->kernel);
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
		static_cast<MacRootKitr*>(user)->onProcLoad(task, path, len);
	});

	this->registerBinaryLoadCallback((void*) this, [] (void *user, void *kext, kmod_info_t *kmod)
	{
		static_cast<MacRootKit*>(user)->onKextLoad(kext, kmod);
	});
}

void MacRootKit::registerEntitlementCallback(void *user, entitlement_callback_t callback)
{
	StoredPair<entitlement_callback_t> *pair = new StoredPair<entitlement_callback_t>;

	pair->first = callback;
	pair->second = user;

	this->entitlementCallbacks.add(pair);
}

void MacRootKit::registerBinaryLoadCallback(void *user, binaryload_callback_t callback)
{
	StoredPair<binaryload_callback_t> *pair = new StoredPair<binaryload_callback_t>;

	pair->first = callback;
	pair->second = user;

	this->binaryLoadedCallbacks.add(pair);
}

void MacRootKit::registerKextLoadCallback(void *user, kextload_callback_t callback)
{
	StoredPair<kextload_callback_t> *pair = new StoredPair<kextload_callback_t>;

	pair->first = callback;
	pair->second = user;

	this->entitlementCallbacks.add(pair);
}

Kext* getKextByIdentifier(char *name)
{
	Array<Kext*> *kexts = this->getKexts();

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

Kext* getKextByAddress(mach_vm_address_t address)
{
	Array<Kext*> *kexts = this->getKexts();

	for(int i = 0; i < kexts->getSize(); i++)
	{
		Kext *kext = kexts->get(i);

		if(strcmp(kext->getAddress(), address) == 0)
		{
			return kext;
		}
	}

	return NULL;
}

void MacRootKit::onEntitlementRequest(task_t task, const char *entitlement, void *original)
{

}

void MacRootKit::onProcLoad(vm_map_t map, const char *path, size_t len)
{

}

void MacRootKit::onKextLoad(void *kext, kmod_info_t *kmod_info)
{
	Kext *loadedKext;

	loadedKext = new Kext(this->getKernel(), kext, kmod_info);
}

kmod_info_t* MacRootKit::findKmodInfo(const char *kextname)
{
	for(kmod_info_t *kmod = *kextKmods; kmod; kmod = kmod->next)
	{
		if(strcmp(kmod->name, kext) == 0)
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

	void *OSKext = lookupKextWithIdentifier(kext);

	return OSKext;
}