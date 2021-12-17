#include "KernelPatcher.hpp"

#include "MacRootKit.hpp"

#include "Arch.hpp"

#include "KernelMachO.hpp"

#include "Hook.hpp"
#include "Payload.hpp"

#include "Task.hpp"
#include "Kernel.hpp"

static KernelPatcher *that = nullptr;

KernelPatcher::KernelPatcher()
{
}

KernelPatcher::KernelPatcher(Kernel *kernel)
{
	that = this;

	this->kernel = kernel;
	this->kextKmods = reinterpret_cast<kmod_info_t**>(this->kernel->getSymbolAddressByName("_kmod"));

	this->installEntitlementHook();
	this->installBinaryLoadHook();
	this->installKextLoadHook();

	// this->installDummyBreakpoint();

	this->processAlreadyLoadedKexts();

	this->waitingForAlreadyLoadedKexts = false;
}

KernelPatcher::~KernelPatcher()
{
}

bool KernelPatcher::dummyBreakpoint(union RegisterState *state)
{
	switch(Arch::getArchitecture())
	{
		case ARCH_x86_64:
			RegisterState_x86_64 *state_x86_64 = &state->state_x86_64;

			break;
		case ARCH_arm64:
			RegisterState_arm64 *state_arm64 = &state->state_arm64;

			break;

		default:
			break;
	}

	return false;
}

Hook* KernelPatcher::installDummyBreakpoint()
{
	Hook *hook;

	mach_vm_address_t mach_msg_trap = this->getKernel()->getSymbolAddressByName("_mach_msg_trap");

	hook = Hook::breakpointForAddress(dynamic_cast<Task*>(this->getKernel()), dynamic_cast<Patcher*>(this), mach_msg_trap);

	hook->addBreakpoint((mach_vm_address_t) KernelPatcher::dummyBreakpoint);

	return hook;
}

void KernelPatcher::onOSKextSaveLoadedKextPanicList()
{
	mach_vm_address_t trampoline;

	if(!that)
		return;

	trampoline = that->getKextLoadHook()->getTrampolineFromChain(reinterpret_cast<mach_vm_address_t>(KernelPatcher::onOSKextSaveLoadedKextPanicList));

	typedef void (*OSKextSaveLoadedKextPanicList)();

	void (*_OSKextSavedLoadedKextPanicList)();

	_OSKextSavedLoadedKextPanicList = reinterpret_cast<OSKextSaveLoadedKextPanicList>(trampoline);

	_OSKextSavedLoadedKextPanicList();

	MAC_RK_LOG("MacRK::OSKextSavedLoadedKextPanicList() hook!\n");

	if(that->waitingForAlreadyLoadedKexts)
	{
		that->processAlreadyLoadedKexts();
		that->waitingForAlreadyLoadedKexts = false;
	} else
	{
		kmod_info_t *kmod = *that->getKextKmods();

		if(kmod)
		{
			that->processKext(kmod, false);
		}
	}
}

void* KernelPatcher::OSKextLookupKextWithIdentifier(const char *identifier)
{
	typedef void* (*lookupKextWithIdentifier)(const char*);

	void* (*__ZN6OSKext24lookupKextWithIdentifierEPKc)(const char*);

	mach_vm_address_t OSKext_lookupWithIdentifier = that->getKernel()->getSymbolAddressByName("__ZN6OSKext24lookupKextWithIdentifierEPKc");

	__ZN6OSKext24lookupKextWithIdentifierEPKc = reinterpret_cast<lookupKextWithIdentifier>(OSKext_lookupWithIdentifier);

	void *OSKext = __ZN6OSKext24lookupKextWithIdentifierEPKc(identifier);

	return OSKext;
}

OSObject* KernelPatcher::copyClientEntitlement(task_t task, const char *entitlement)
{
	Hook *hook = that->getEntitlementHook();

	mach_vm_address_t trampoline;

	MAC_RK_LOG("MacRK::KernelPatcher::copyClientEntitlement() hook!\n");

	trampoline = hook->getTrampolineFromChain(reinterpret_cast<mach_vm_address_t>(KernelPatcher::copyClientEntitlement));

	typedef OSObject* (*origCopyClientEntitlement)(task_t, const char*);

	OSObject *original = reinterpret_cast<origCopyClientEntitlement>(trampoline)(task, entitlement);

	if(strcmp(entitlement, "com.apple.private.audio.driver-host") == 0)
	{
		original = OSBoolean::withBoolean(true);
	}

	if(that)
	{
		StoredArray<MacRootKit::entitlement_callback_t> *entitlementCallbacks;

		MacRootKit *rootkit = that->getKernel()->getRootKit();

		entitlementCallbacks = rootkit->getEntitlementCallbacks();

		for(int i = 0; i < entitlementCallbacks->getSize(); i++)
		{
			auto handler = entitlementCallbacks->get(i);

			MacRootKit::entitlement_callback_t callback = handler->first;

			void *user = handler->second;

			callback(user, task, entitlement, (void*) original);
		}
	}

	return original;
}

void KernelPatcher::taskSetMainThreadQos(task_t task, thread_t thread)
{
	Hook *hook = that->getBinaryLoadHook();

	mach_vm_address_t trampoline;

	trampoline = hook->getTrampolineFromChain(reinterpret_cast<mach_vm_address_t>(KernelPatcher::taskSetMainThreadQos));

	typedef void *(*task_set_main_thread_qos)(task_t, thread_t);

	MAC_RK_LOG("MacRK::task_set_main_thread_qos hook!\n");

	if(that)
	{
		StoredArray<MacRootKit::binaryload_callback_t> *binaryLoadCallbacks;

		MacRootKit *rootkit = that->getKernel()->getRootKit();

		binaryLoadCallbacks = rootkit->getBinaryLoadCallbacks();

		for(int i = 0; i < binaryLoadCallbacks->getSize(); i++)
		{
			auto handler = binaryLoadCallbacks->get(i);

			MacRootKit::binaryload_callback_t callback = handler->first;

			void *user = handler->second;

			// callback(user, task, thread);
		}
	}

	reinterpret_cast<task_set_main_thread_qos>(trampoline)(task, thread);
}

void KernelPatcher::findAndReplace(void *data, size_t data_size,
									const void *find, size_t find_size,
									const void *replace, size_t replace_size)
{
	void *res;
}

void KernelPatcher::routeFunction(Hook *hook)
{
}

void KernelPatcher::onKextLoad(void *kext, kmod_info_t *kmod)
{
	Kext::onKextLoad(kext, kmod);
}

void KernelPatcher::onExec(task_t task, const char *path, size_t len)
{
}

void KernelPatcher::onEntitlementRequest(task_t task, const char *entitlement, void *original)
{
}

Hook* KernelPatcher::installEntitlementHook()
{
	Hook *hook;

	mach_vm_address_t orig_copyClientEntitlement;
	mach_vm_address_t hooked_copyClientEntitlement;

	orig_copyClientEntitlement = this->getKernel()->getSymbolAddressByName("__ZN12IOUserClient21copyClientEntitlementEP4taskPKc");

	hooked_copyClientEntitlement = reinterpret_cast<mach_vm_address_t>(KernelPatcher::copyClientEntitlement);

	hook = Hook::hookForFunction(this->getKernel(), this, orig_copyClientEntitlement);

	this->installHook(hook, hooked_copyClientEntitlement);

	this->entitlementHook = hook;

	return hook;
}

Hook* KernelPatcher::installBinaryLoadHook()
{
	Hook *hook;

	mach_vm_address_t orig_task_set_main_thread_qos;
	mach_vm_address_t hooked_task_set_main_thread_qos;

	orig_task_set_main_thread_qos = this->getKernel()->getSymbolAddressByName("_task_main_thread_qos");

	hooked_task_set_main_thread_qos = reinterpret_cast<mach_vm_address_t>(KernelPatcher::taskSetMainThreadQos);

	hook = Hook::hookForFunction(this->getKernel(), this, orig_task_set_main_thread_qos);

	this->installHook(hook, hooked_task_set_main_thread_qos);

	this->binaryLoadHook = hook;

	return hook;
}

Hook* KernelPatcher::installKextLoadHook()
{
	Hook *hook;

	mach_vm_address_t orig_OSKextSaveLoadedKextPanicList;
	mach_vm_address_t hooked_OSKextSaveLoadedKextPanicList;

	orig_OSKextSaveLoadedKextPanicList = this->getKernel()->getSymbolAddressByName("__ZN6OSKext24lookupKextWithIdentifierEPKc");

	hooked_OSKextSaveLoadedKextPanicList = reinterpret_cast<mach_vm_address_t>(KernelPatcher::onOSKextSaveLoadedKextPanicList);

	hook = Hook::hookForFunction(this->getKernel(), this, orig_OSKextSaveLoadedKextPanicList);

	this->installHook(hook, hooked_OSKextSaveLoadedKextPanicList);

	this->kextLoadHook = hook;

	return hook;
}

void KernelPatcher::registerCallbacks()
{
	MacRootKit *rootkit = this->getKernel()->getRootKit();

	rootkit->registerEntitlementCallback((void*) this, [] (void *user, task_t task, const char *entitlement, void *original)
	{
		static_cast<KernelPatcher*>(user)->onEntitlementRequest(task, entitlement, original);
	});

	rootkit->registerBinaryLoadCallback((void*) this, [] (void *user, task_t task, const char *path, size_t len)
	{
		static_cast<KernelPatcher*>(user)->onExec(task, path, len);
	});

	rootkit->registerKextLoadCallback((void*) this, [] (void *user, void *kext, kmod_info_t *kmod)
	{
		static_cast<KernelPatcher*>(user)->onKextLoad(kext, kmod);
	});
}

void KernelPatcher::processAlreadyLoadedKexts()
{
	for(kmod_info_t *kmod = *kextKmods; kmod; kmod = kmod->next)
		processKext(kmod, true);
}

void KernelPatcher::processKext(kmod_info_t *kmod, bool loaded)
{
	MacRootKit *rootkit;

	void *OSKext;

	StoredArray<MacRootKit::kextload_callback_t> *kextLoadCallbacks;

	mach_vm_address_t kmod_address = (mach_vm_address_t) kmod->address;

	rootkit = this->getKernel()->getRootKit();

	kextLoadCallbacks = rootkit->getKextLoadCallbacks();

	OSKext = KernelPatcher::OSKextLookupKextWithIdentifier(static_cast<char*>(kmod->name));

	for(int i = 0; i < kextLoadCallbacks->getSize(); i++)
	{
		auto handler = kextLoadCallbacks->get(i);

		MacRootKit::kextload_callback_t callback = handler->first;

		void *user = handler->second;

		callback(user, OSKext, kmod);
	}
}

mach_vm_address_t KernelPatcher::injectPayload(mach_vm_address_t address, Payload *payload)
{
	return (mach_vm_address_t) 0;
}

mach_vm_address_t KernelPatcher::injectSegment(mach_vm_address_t address, Payload *payload)
{
	return (mach_vm_address_t) 0;
}

void KernelPatcher::applyKernelPatch(KernelPatch *patch)
{
}

void KernelPatcher::applyKextPatch(KextPatch *patch)
{
}

void KernelPatcher::removeKernelPatch(KernelPatch *patch)
{
}

void KernelPatcher::removeKextPatch(KextPatch *patch)
{
}