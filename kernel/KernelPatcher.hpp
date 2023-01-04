#ifndef __KERNEL_PATCHER_HPP_
#define __KERNEL_PATCHER_HPP_

#include <IOKit/IOLib.h>

#include <mach/mach_types.h>
#include <mach/kmod.h>

#include "Patcher.hpp"
#include "Arch.hpp"

namespace xnu
{
	class Kernel;
	class Kext;
}

class MachO;
class Symbol;

struct KextPatch
{
	public:
		xnu::Kext *kext;

		MachO *macho;
		Symbol *symbol;

		const uint8_t *find;
		const uint8_t *replace;

		size_t size;
		size_t count;

		off_t offset;
};

struct KernelPatch
{
	public:
		xnu::Kernel *kernel;

		MachO *macho;
		Symbol *symbol;

		const uint8_t *find;
		const uint8_t *replace;

		size_t size;
		size_t count;

		off_t offset;
};

namespace mrk
{
	class Hook;
	class Payload;

	class KernelPatcher : public mrk::Patcher
	{
		public:
			KernelPatcher();
			KernelPatcher(xnu::Kernel *kernel);

			~KernelPatcher();

			xnu::Kernel* getKernel() { return kernel; }

			kmod_info_t** getKextKmods() { return kextKmods; }

			mrk::Hook* getEntitlementHook() { return entitlementHook; }
			mrk::Hook* getBinaryLoadHook() { return binaryLoadHook; }
			mrk::Hook* getKextLoadHook() { return kextLoadHook; }

			static bool dummyBreakpoint(union Arch::RegisterState *state);

			static void onOSKextSaveLoadedKextPanicList();

			static void* OSKextLookupKextWithIdentifier(const char *identifier);

			static OSObject* copyClientEntitlement(task_t task, const char *entitlement);

			static void taskSetMainThreadQos(task_t task, thread_t thread);

			virtual void findAndReplace(void *data, size_t data_size,
										const void *find, size_t find_size,
										const void *replace, size_t replace_size);

			virtual void routeFunction(mrk::Hook *hook);

			virtual void onKextLoad(void *kext, kmod_info_t *kmod);

			virtual void onExec(task_t task, const char *path, size_t len);

			virtual void onEntitlementRequest(task_t task, const char *entitlement, void *original);

			mrk::Hook* installDummyBreakpoint();

			mrk::Hook* installEntitlementHook();
			mrk::Hook* installBinaryLoadHook();
			mrk::Hook* installKextLoadHook();

			void registerCallbacks();

			void processAlreadyLoadedKexts();

			void processKext(kmod_info_t *kmod, bool loaded);

			mach_vm_address_t injectPayload(mach_vm_address_t address, mrk::Payload *payload);

			mach_vm_address_t injectSegment(mach_vm_address_t address, mrk::Payload *payload);

			void applyKernelPatch(struct KernelPatch *patch);
			void applyKextPatch(struct KextPatch *patch);

			void patchPmapEnterOptions();

			void removeKernelPatch(struct KernelPatch *patch);
			void removeKextPatch(struct KextPatch *patch);

		private:
			xnu::Kernel *kernel;

			kmod_info_t **kextKmods;

			mrk::Hook *entitlementHook;
			mrk::Hook *binaryLoadHook;
			mrk::Hook *kextLoadHook;

			bool waitingForAlreadyLoadedKexts = false;

			std::Array<struct KernelPatch*> kernelPatches;
			std::Array<struct KextPatch*> kextPatches;
	};

};


#endif