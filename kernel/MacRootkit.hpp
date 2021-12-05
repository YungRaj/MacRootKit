#ifndef __MACRootKit_HPP_
#define __MACRootKit_HPP_

#include "Kernel.hpp"
#include "Kext.hpp"

class IOKernelRootKitService;

class Kernel;
class Kext;

template <typename T, typename Y=void *>
using StoredPair = Pair<T, Y>;

template <typename T, typename Y=void *>
using StoredArray = Array<StoredPair<T, Y>*>;

class MacRootKit
{
	using entitlement_callback_t = void (*)(void *user, task_t task, const char *entitlement, OSObject *original);

	using binaryload_callback_t = void (*)(void *user, vm_map_t map, const char *path, size_t len);

	using kextload_callback_t void (*)(void *user, Kext *kext, char *kextname);

	public:
		MacRootKit();

		~MacRootKit();

		Kernel* getKernel() { return kernel; }

		enum Architectures getPlatformArchitectures() { return platformArchitecture; }

		Kext* getKextByIdentifier();

		Kext* getKextByAddress(mach_vm_address_t address);

		KernelPatcher* getKernelPatcher() { return kernelPatcher; }

		Array<entitlement_callback_t>* getEntitlementCallbacks() { return &entitlementCallbacks; }

		Array<binaryload_callback_t>* getBinaryLoadCallbacks() { return &binaryLoadCallbacks; }

		Array<kextload_callback_t>* getKextLoadCallbacks() { return &kextLoadCallbacks; }

		void registerEntitlementCallback(void *user, entitlement_callback_t callback);

		void registerBinaryLoadCallback(void *user, binaryload_callback_t callback);

		void registerKextLoadCallback(void *user, kextload_callback_t callback);

		void onEntitlementRequest(task_t task, const char *entitlement, OSObject *original);

		void onProcLoad(vm_map_t map, const char *path, size_t len);

		void onKextLoad(Kext *kext, char *kextname);

		kmod_info_t* findKmodInfo(const char *kextname);

		void* findOSKextByIdentifier(const char *kextidentifier);

	private:
		Kernel *kernel;

		KernelPatcher *kernelPatcher;

		enum Architectures platformArchitecture;

		bool waitingForAlreadyLoadedKexts;

		Array<Kext*> kexts;

		StoredArray<entitlement_callback_t> entitlementCallbacks;

		StoredArray<binaryload_callback_t> binaryLoadCallbacks;

		StoredArray<kextload_callback_t> kextLoadCallbacks;
};

#endif