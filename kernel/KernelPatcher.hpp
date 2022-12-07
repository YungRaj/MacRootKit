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

using namespace Arch;
using namespace xnu;

class MachO;
class Symbol;

struct KextPatch
{
	public:
		Kext *kext;

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
		Kernel *kernel;

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

	class KernelPatcher : public Patcher
	{
		public:
			KernelPatcher();
			KernelPatcher(Kernel *kernel);

			~KernelPatcher();

			Kernel* getKernel() { return kernel; }

			kmod_info_t** getKextKmods() { return kextKmods; }

			Hook* getEntitlementHook() { return entitlementHook; }
			Hook* getBinaryLoadHook() { return binaryLoadHook; }
			Hook* getKextLoadHook() { return kextLoadHook; }

			static bool dummyBreakpoint(union RegisterState *state);

			static void onOSKextSaveLoadedKextPanicList();

			static void* OSKextLookupKextWithIdentifier(const char *identifier);

			static OSObject* copyClientEntitlement(task_t task, const char *entitlement);

			static void taskSetMainThreadQos(task_t task, thread_t thread);

			virtual void findAndReplace(void *data, size_t data_size,
										const void *find, size_t find_size,
										const void *replace, size_t replace_size);

			virtual void routeFunction(Hook *hook);

			virtual void onKextLoad(void *kext, kmod_info_t *kmod);

			virtual void onExec(task_t task, const char *path, size_t len);

			virtual void onEntitlementRequest(task_t task, const char *entitlement, void *original);

			Hook* installDummyBreakpoint();

			Hook* installEntitlementHook();
			Hook* installBinaryLoadHook();
			Hook* installKextLoadHook();

			void registerCallbacks();

			void processAlreadyLoadedKexts();

			void processKext(kmod_info_t *kmod, bool loaded);

			mach_vm_address_t injectPayload(mach_vm_address_t address, Payload *payload);

			mach_vm_address_t injectSegment(mach_vm_address_t address, Payload *payload);

			void applyKernelPatch(struct KernelPatch *patch);
			void applyKextPatch(struct KextPatch *patch);

			void patchPmapEnterOptions();

			void removeKernelPatch(struct KernelPatch *patch);
			void removeKextPatch(struct KextPatch *patch);

		private:
			Kernel *kernel;

			kmod_info_t **kextKmods;

			Hook *entitlementHook;
			Hook *binaryLoadHook;
			Hook *kextLoadHook;

			bool waitingForAlreadyLoadedKexts = false;

			Array<struct KernelPatch*> kernelPatches;
			Array<struct KextPatch*> kextPatches;
	};

};


#endif