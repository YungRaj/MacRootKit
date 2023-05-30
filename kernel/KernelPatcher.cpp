#include "KernelPatcher.hpp"

#include "MacRootKit.hpp"

#include "Arch.hpp"

#include "KernelMachO.hpp"

#include "Hook.hpp"
#include "Payload.hpp"

#include "Task.hpp"
#include "Kernel.hpp"

#include "Disassembler.hpp"

#ifdef __arm64__

#include <arm64/PatchFinder_arm64.hpp>

using namespace Arch::arm64::PatchFinder;

#elif __x86_64__

#include <x86_64/PatchFinder_x86_64.hpp>

using namespace Arch::x86_64::PatchFinder;

#endif

using namespace Arch;
using namespace mrk;

static KernelPatcher *that = nullptr;

KernelPatcher::KernelPatcher()
{
}

KernelPatcher::KernelPatcher(xnu::Kernel *kernel)
{
	that = this;

	this->kernel = kernel;
	this->kextKmods = reinterpret_cast<kmod_info_t**>(this->kernel->getSymbolAddressByName("_kmod"));

	this->processAlreadyLoadedKexts();

	this->waitingForAlreadyLoadedKexts = false;

	this->installEntitlementHook();

#ifdef __x86_64__
	// binary load hook does not work on arm64 because symbol to hook does not exist
	this->installBinaryLoadHook();

	// kext load hook does not work on arm64 because symbol to hook does not exist
	this->installKextLoadHook();
#endif

	// this->installDummyBreakpoint();
}

KernelPatcher::~KernelPatcher()
{
}

bool KernelPatcher::dummyBreakpoint(union Arch::RegisterState *state)
{
	RegisterState_x86_64 *state_x86_64;
	RegisterState_arm64  *state_arm64;

	switch(Arch::getCurrentArchitecture())
	{
		case ARCH_x86_64:
			state_x86_64 = &state->state_x86_64;

			break;
		case ARCH_arm64:
			state_arm64 = &state->state_arm64;

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
	
	#ifdef __x86_64__
		kmod_info_t *kmod = *that->getKextKmods();

		if(kmod)
		{
			that->processKext(kmod, false);
		}
	#endif
	
	}
}

void* KernelPatcher::OSKextLookupKextWithIdentifier(const char *identifier)
{
	typedef void* (*lookupKextWithIdentifier)(const char*);

	void* (*__ZN6OSKext24lookupKextWithIdentifierEPKc)(const char*);

	mach_vm_address_t OSKext_lookupWithIdentifier = that->getKernel()->getSymbolAddressByName("__ZN6OSKext24lookupKextWithIdentifierEPKc");

	__ZN6OSKext24lookupKextWithIdentifierEPKc = reinterpret_cast<lookupKextWithIdentifier>(OSKext_lookupWithIdentifier);

#ifdef _x86_64__
	void *OSKext = __ZN6OSKext24lookupKextWithIdentifierEPKc(identifier);

	return OSKext;
#elif __arm64__
	return 0;
#endif
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

	if(strcmp(entitlement, "com.apple.security.app-sandbox") == 0)
	{
		original = OSBoolean::withBoolean(false);
	}

	if(strcmp(entitlement, "com.apple.private.FairPlayIOKitUserClient.access") == 0)
	{
		original = OSBoolean::withBoolean(true);
	}

	if(strcmp(entitlement, "com.apple.private.ProvInfoIOKitUserClient.access") == 0)
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

	char buffer[128];

	snprintf(buffer, 128, "0x%llx", orig_copyClientEntitlement);

	MAC_RK_LOG("MacRK::__ZN12IOUserClient21copyClientEntitlementEP4taskPKc = %s\n", buffer);

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
#ifdef __x86_64__

	for(kmod_info_t *kmod = *kextKmods; kmod; kmod = kmod->next)
	{
		if(kmod->address && kmod->size)
		{
			char buffer1[128];
			char buffer2[128];

			snprintf(buffer1, 128, "0x%lx", kmod->address);
			snprintf(buffer2, 128, "0x%x", *(uint32_t*) kmod->address);

			MAC_RK_LOG("MacRK::KernelPatcher::processing Kext %s = %s @ %s\n", (char*) kmod->name, buffer1, buffer2);
		
			this->processKext(kmod, true);
		}
	}
	
#endif

#ifdef __arm64__

	mach_vm_address_t kernel_cache = Kernel::findKernelCache();

	struct mach_header_64 *mh = reinterpret_cast<struct mach_header_64*>(kernel_cache);

	uint8_t *q = reinterpret_cast<uint8_t*>(mh) + sizeof(struct mach_header_64);

	for(int i = 0; i < mh->ncmds; i++)
	{
		struct load_command *load_command = reinterpret_cast<struct load_command*>(q);

		if(load_command->cmd == LC_FILESET_ENTRY)
		{
			struct fileset_entry_command *fileset_entry_command = reinterpret_cast<struct fileset_entry_command*>(load_command);

			mach_vm_address_t base = fileset_entry_command->vmaddr;

			char *entry_id = reinterpret_cast<char*>(fileset_entry_command) + fileset_entry_command->entry_id;

			if(base && strcmp(entry_id, "com.apple.kernel") != 0)
			{
				kmod_info_t *kmod = new kmod_info_t;

				kmod->address = 0xfffffe0000000000 | base;
				kmod->size = 0;

				strlcpy(reinterpret_cast<char*>(&kmod->name), entry_id, strlen(entry_id) + 1);

				kmod->start = (kmod_start_func_t*) 0;
				kmod->stop = (kmod_stop_func_t*) 0;

				this->processKext(kmod, true);

				char buffer1[128];
				char buffer2[128];

				snprintf(buffer1, 128, "0x%lx", kmod->address);
				snprintf(buffer2, 128, "0x%x", *(uint32_t*) kmod->address);

				MAC_RK_LOG("MacRK::KernelPatcher::processing Kext %s = %s @ %s = %s\n", entry_id, buffer1, entry_id, buffer2);
			}
		}

		q += load_command->cmdsize;
	}

#endif

	this->waitingForAlreadyLoadedKexts = false;
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

void KernelPatcher::patchPmapEnterOptions()
{
	using namespace Arch::arm64;

	xnu::Kernel *kernel = this->kernel;

	MachO *macho = kernel->getMachO();

	mach_vm_address_t vm_allocate_external = kernel->getSymbolAddressByName("_vm_allocate_external");

	char buffer[128];

	mach_vm_address_t branch = Arch::arm64::PatchFinder::step64(macho, vm_allocate_external, 0x10, reinterpret_cast<bool(*)(uint32_t*)>(Arch::arm64::is_b), -1, -1);

	bool sign;

	b_t b = *(b_t*) branch;

	uint64_t imm = b.imm;

	if(imm & 0x2000000)
	{
		imm = ~(imm - 1);
		imm &= 0x1FFFFFF;

		sign = true;
	} else
	{
		sign = false;
	}

	imm *= (1 << 2);

	mach_vm_address_t vm_allocate = sign ? branch - imm : branch + imm;

	branch = Arch::arm64::PatchFinder::step64(macho, vm_allocate, 0x100, reinterpret_cast<bool(*)(uint32_t*)>(Arch::arm64::is_bl), -1, -1);

	bl_t bl = *(bl_t*) branch;

	imm = bl.imm;

	if(imm & 0x2000000)
	{
		imm = ~(imm - 1);
		imm &= 0x1FFFFFF;

		sign = true;
	} else
	{
		sign = false;
	}

	imm *= (1 << 2);

	uint32_t nop = 0xd503201f;

	mach_vm_address_t vm_map_enter = sign ? branch - imm : branch + imm;

	mach_vm_address_t pmap_enter_options_strref = Arch::arm64::PatchFinder::findStringReference(macho, "pmap_enter_options(): attempt to add executable mapping to kernel_pmap @%s:%d", 1, __cstring_, __TEXT_PPL_BASE, false);

	mach_vm_address_t pmap_enter_options = Arch::arm64::PatchFinder::findFunctionBegin(macho, pmap_enter_options_strref - 0xFFF, pmap_enter_options_strref);

	mach_vm_address_t panic = Arch::arm64::PatchFinder::stepBack64(macho, pmap_enter_options_strref - sizeof(uint32_t) * 2, 0x20, reinterpret_cast<bool(*)(uint32_t*)>(Arch::arm64::is_adrp), -1, -1);

	mach_vm_address_t panic_xref = Arch::arm64::PatchFinder::xref64(macho, panic - 0xFFF, panic - sizeof(uint32_t), panic);

	branch = Arch::arm64::PatchFinder::stepBack64(macho, panic_xref - sizeof(uint32_t), 0x10, reinterpret_cast<bool(*)(uint32_t*)>(Arch::arm64::is_b_cond), -1, -1);

	kernel->write(branch, (void*) &nop, sizeof(nop));

	branch = Arch::arm64::PatchFinder::stepBack64(macho, branch - sizeof(uint32_t), 0x20, reinterpret_cast<bool(*)(uint32_t*)>(Arch::arm64::is_b_cond), -1, -1);

	kernel->write(branch, (void*) &nop, sizeof(nop));

	branch = Arch::arm64::PatchFinder::stepBack64(macho, branch - sizeof(uint32_t), 0x10, reinterpret_cast<bool(*)(uint32_t*)>(Arch::arm64::is_b_cond), -1, -1);

	kernel->write(branch, (void*) &nop, sizeof(nop));

	uint32_t mov_x26_0x7 = 0xd28000fa;

	kernel->write(panic_xref - sizeof(uint32_t) * 2, (void*) &mov_x26_0x7, sizeof(mov_x26_0x7));

	kernel->write(panic_xref - sizeof(uint32_t), (void*) &nop, sizeof(nop));

	kernel->write(panic_xref + sizeof(uint32_t), (void*) &nop, sizeof(nop));

	// uint64_t breakpoint = 0xD4388E40D4388E40;

	//this->write(vm_map_enter, (void*) &breakpoint, sizeof(uint64_t));

	// MAC_RK_LOG("MacRK::@ vm_map_enter = 0x%x\n", *(uint32_t*) vm_map_enter);
}

void KernelPatcher::applyKernelPatch(struct KernelPatch *patch)
{
	xnu::Kernel *kernel;

	MachO *macho;

	Symbol *symbol;

	const uint8_t *find;
	const uint8_t *replace;

	size_t size;
	size_t count;

	off_t offset;

	kernel = patch->kernel;
	macho = patch->macho;

	find = patch->find;
	replace = patch->replace;


	size = patch->size;
	count = patch->count;

	offset = patch->offset;

	if(!symbol)
	{
		// patch everything you can N times;

		mach_vm_address_t base = kernel->getBase();

		mach_vm_address_t current_address = base;

		size_t size = macho->getSize();

		for(int i = 0; current_address < base + size && (i < count || count == 0); i++)
		{
			while(current_address < base + size && memcmp((void*) current_address, (void*) find, size) != 0)
			{
				current_address++;
			}

			if(current_address != base + size)
			{
				kernel->write(current_address, (void*) replace, size);
			}
		}

	} else
	{
		// patch the function directed by symbol

		mach_vm_address_t address = symbol->getAddress();

		if(find)
		{
			// search up to N bytes from beginning of function
			// use PatchFinder::findFunctionEnd() to get ending point

			mach_vm_address_t current_address = address;

			for(int i = 0; i < 0x400; i++)
			{
				if(memcmp((void*) current_address, (void*) find, size) == 0)
				{
					kernel->write(current_address, (void*) replace, size);
				}

				current_address++;
			}
		} else
		{
			// use offset provided by user to patch bytes in function

			kernel->write(address + offset, (void*) replace, size);
		}
	}

	this->kernelPatches.add(patch);
}

void KernelPatcher::applyKextPatch(struct KextPatch *patch)
{
	Kext *kext;

	MachO *macho;

	Symbol *symbol;

	const uint8_t *find;
	const uint8_t *replace;

	size_t size;
	size_t count;

	off_t offset;

	kext = patch->kext;
	macho = patch->macho;

	find = patch->find;
	replace = patch->replace;


	size = patch->size;
	count = patch->count;

	offset = patch->offset;

	if(!symbol)
	{
		// patch everything you can N times;

		mach_vm_address_t base = kext->getBase();

		mach_vm_address_t current_address = base;

		size_t size = macho->getSize();

		for(int i = 0; current_address < base + size && (i < count || count == 0); i++)
		{
			while(current_address < base + size && memcmp((void*) current_address, (void*) find, size) != 0)
			{
				current_address++;
			}

			if(current_address != base + size)
			{
				kernel->write(current_address, (void*) replace, size);
			}
		}

	} else
	{
		// patch the function directed by symbol

		mach_vm_address_t address = symbol->getAddress();

		if(find)
		{
			// search up to N bytes from beginning of function
			// use PatchFinder::findFunctionEnd() to get ending point

			mach_vm_address_t current_address = address;

			for(int i = 0; i < 0x400; i++)
			{
				if(memcmp((void*) current_address, (void*) find, size) == 0)
				{
					kernel->write(current_address, (void*) replace, size);
				}

				current_address++;
			}
		} else
		{
			// use offset provided by user to patch bytes in function

			kernel->write(address + offset, (void*) replace, size);
		}
	}

	this->kextPatches.add(patch);
}

void KernelPatcher::removeKernelPatch(struct KernelPatch *patch)
{
	xnu::Kernel *kernel;

	MachO *macho;

	Symbol *symbol;

	const uint8_t *find;
	const uint8_t *replace;

	size_t size;
	size_t count;

	off_t offset;

	kernel = patch->kernel;
	macho = patch->macho;

	find = patch->find;
	replace = patch->replace;


	size = patch->size;
	count = patch->count;

	offset = patch->offset;

	if(!symbol)
	{
		// patch everything you can N times;

		mach_vm_address_t base = kernel->getBase();

		mach_vm_address_t current_address = base;

		size_t size = macho->getSize();

		for(int i = 0; current_address < base + size && (i < count || count == 0); i++)
		{
			while(current_address < base + size && memcmp((void*) current_address, (void*) replace, size) != 0)
			{
				current_address++;
			}

			if(current_address != base + size)
			{
				kernel->write(current_address, (void*) find, size);
			}
		}

	} else
	{
		// patch the function directed by symbol

		mach_vm_address_t address = symbol->getAddress();

		if(find)
		{
			// search up to N bytes from beginning of function
			// use PatchFinder::findFunctionEnd() to get ending point

			mach_vm_address_t current_address = address;

			for(int i = 0; i < 0x400; i++)
			{
				if(memcmp((void*) current_address, (void*) replace, size) == 0)
				{
					kernel->write(current_address, (void*) find, size);
				}

				current_address++;
			}
		} else
		{
			// use offset provided by user to patch bytes in function

			kernel->write(address + offset, (void*) find, size);
		}
	}

	this->kernelPatches.add(patch);
}

void KernelPatcher::removeKextPatch(struct KextPatch *patch)
{
	Kext *kext;

	MachO *macho;

	Symbol *symbol;

	const uint8_t *find;
	const uint8_t *replace;

	size_t size;
	size_t count;

	off_t offset;

	kext = patch->kext;
	macho = patch->macho;

	find = patch->find;
	replace = patch->replace;


	size = patch->size;
	count = patch->count;

	offset = patch->offset;

	if(!symbol)
	{
		// patch everything you can N times;

		mach_vm_address_t base = kext->getBase();

		mach_vm_address_t current_address = base;

		size_t size = macho->getSize();

		for(int i = 0; current_address < base + size && (i < count || count == 0); i++)
		{
			while(current_address < base + size && memcmp((void*) current_address, (void*) replace, size) != 0)
			{
				current_address++;
			}

			if(current_address != base + size)
			{
				kernel->write(current_address, (void*) find, size);
			}
		}

	} else
	{
		// patch the function directed by symbol

		mach_vm_address_t address = symbol->getAddress();

		if(find)
		{
			// search up to N bytes from beginning of function
			// use PatchFinder::findFunctionEnd() to get ending point

			mach_vm_address_t current_address = address;

			for(int i = 0; i < 0x400; i++)
			{
				if(memcmp((void*) current_address, (void*) replace, size) == 0)
				{
					kernel->write(current_address, (void*) find, size);
				}

				current_address++;
			}
		} else
		{
			// use offset provided by user to patch bytes in function

			kernel->write(address + offset, (void*) find, size);
		}
	}

	this->kextPatches.add(patch);
}