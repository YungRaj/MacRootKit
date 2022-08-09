#ifndef __MACROOTKIT_HPP_
#define __MACROOTKIT_HPP_

#include "Arch.hpp"

#include "Kernel.hpp"
#include "KernelPatcher.hpp"

#include "Kext.hpp"

#include <string.h>

using namespace Arch;

class IOKernelRootKitService;

class Kernel;
class Kext;

template <typename T, typename Y=void *>
using StoredPair = Pair<T, Y>;

template <typename T, typename Y=void *>
using StoredArray = Array<StoredPair<T, Y>*>;

class MacRootKit
{
	public:
		using entitlement_callback_t = void (*)(void *user, task_t task, const char *entitlement, void *original);

		using binaryload_callback_t = void (*)(void *user, task_t task, const char *path, size_t len);

		using kextload_callback_t = void (*)(void *user, void *kext, kmod_info_t *kmod_info);

	public:
		MacRootKit(Kernel *kernel);

		~MacRootKit();

		Kernel* getKernel() { return kernel; }

		Architecture* getArchitecture() { return architecture; }

		enum Architectures getPlatformArchitecture() { return platformArchitecture; }

		Array<Kext*>* getKexts() { return &kexts; }

		Kext* getKextByIdentifier(char *name);

		Kext* getKextByAddress(mach_vm_address_t address);

		KernelPatcher* getKernelPatcher() { return kernelPatcher; }

		StoredArray<entitlement_callback_t>* getEntitlementCallbacks() { return &entitlementCallbacks; }

		StoredArray<binaryload_callback_t>* getBinaryLoadCallbacks() { return &binaryLoadCallbacks; }

		StoredArray<kextload_callback_t>* getKextLoadCallbacks() { return &kextLoadCallbacks; }

		void registerCallbacks();

		void registerEntitlementCallback(void *user, entitlement_callback_t callback);

		void registerBinaryLoadCallback(void *user, binaryload_callback_t callback);

		void registerKextLoadCallback(void *user, kextload_callback_t callback);

		void onEntitlementRequest(task_t task, const char *entitlement, void *original);

		void onProcLoad(task_t task, const char *path, size_t len);

		void onKextLoad(void *kext, kmod_info_t *kmod);

		kmod_info_t* findKmodInfo(const char *kextname);

		void* findOSKextByIdentifier(const char *kextidentifier);

	private:
		Architecture *architecture;

		Kernel *kernel;

		KernelPatcher *kernelPatcher;

		enum Architectures platformArchitecture;

		bool waitingForAlreadyLoadedKexts;

		Array<Kext*> kexts;

		kmod_info_t **kextKmods;

		StoredArray<entitlement_callback_t> entitlementCallbacks;

		StoredArray<binaryload_callback_t> binaryLoadCallbacks;

		StoredArray<kextload_callback_t> kextLoadCallbacks;
};

#endif