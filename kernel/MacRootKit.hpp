#ifndef __MACROOTKIT_HPP_
#define __MACROOTKIT_HPP_

#include "Arch.hpp"

#include "Kernel.hpp"
#include "KernelPatcher.hpp"

#include "Kext.hpp"

#include <string.h>

namespace xnu
{
	class Kext;
	class Kernel;
}

using namespace xnu;

class IOKernelRootKitService;

namespace mrk
{
	class Hook;

	template <typename T, typename Y=void *>
	using StoredPair = Pair<T, Y>;

	template <typename T, typename Y=void *>
	using StoredArray = std::Array<StoredPair<T, Y>*>;

	class MacRootKit
	{
		public:
			using entitlement_callback_t = void (*)(void *user, task_t task, const char *entitlement, void *original);

			using binaryload_callback_t = void (*)(void *user, task_t task, const char *path, size_t len);

			using kextload_callback_t = void (*)(void *user, void *kext, kmod_info_t *kmod_info);

		public:
			MacRootKit(xnu::Kernel *kernel);

			~MacRootKit();

			xnu::Kernel* getKernel() { return kernel; }

			Arch::Architecture* getArchitecture() { return architecture; }

			enum Arch::Architectures getPlatformArchitecture() { return platformArchitecture; }

			std::Array<xnu::Kext*>* getKexts() { return &kexts; }

			xnu::Kext* getKextByIdentifier(char *name);

			xnu::Kext* getKextByAddress(mach_vm_address_t address);

			mrk::KernelPatcher* getKernelPatcher() { return kernelPatcher; }

			mrk::Plugin* getPlugin(const char *pluginName)
			{
				for(size_t i = 0; i < plugins.getSize(); i++)
				{
					Plugin *plugin = plugins.get(i);

					if(strcmp(plugin->getProduct(), pluginName) == 0)
						return plugin;
				}

				return NULL;
			}

			void installPlugin(mrk::Plugin *plugin) { this->plugins.add(plugin); }

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
			Arch::Architecture *architecture;

			xnu::Kernel *kernel;

			mrk::KernelPatcher *kernelPatcher;

			enum Arch::Architectures platformArchitecture;

			bool waitingForAlreadyLoadedKexts;

			std::Array<xnu::Kext*> kexts;

			kmod_info_t **kextKmods;

			Array<mrk::Plugin*> plugins;

			StoredArray<entitlement_callback_t> entitlementCallbacks;

			StoredArray<binaryload_callback_t> binaryLoadCallbacks;

			StoredArray<kextload_callback_t> kextLoadCallbacks;
	};
}

#endif