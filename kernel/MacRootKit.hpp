/*
 * Copyright (c) YungRaj
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <Types.h>

#include "Arch.hpp"

#include "Kernel.hpp"
#include "KernelPatcher.hpp"

#include "Plugin.hpp"

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
	using StoredArray = std::vector<StoredPair<T, Y>*>;

	class MacRootKit
	{
		public:
			using entitlement_callback_t = void (*)(void *user, task_t task, const char *entitlement, void *original);

			using binaryload_callback_t = void (*)(void *user, task_t task, const char *path, Size len);

			using kextload_callback_t = void (*)(void *user, void *kext, xnu::KmodInfo *kmod_info);

		public:
			MacRootKit(xnu::Kernel *kernel);

			~MacRootKit();

			xnu::Kernel* getKernel() { return kernel; }

			Arch::Architecture* getArchitecture() { return architecture; }

			enum Arch::Architectures getPlatformArchitecture() { return platformArchitecture; }

			std::vector<xnu::Kext*>& getKexts() { return kexts; }

			xnu::Kext* getKextByIdentifier(char *name);

			xnu::Kext* getKextByAddress(xnu::Mach::VmAddress address);

			mrk::KernelPatcher* getKernelPatcher() { return kernelPatcher; }

			mrk::Plugin* getPlugin(const char *pluginName)
			{
				for(int i = 0; i < plugins.size(); i++)
				{
					mrk::Plugin *plugin = plugins.at(i);

					if(strcmp(plugin->getProduct(), pluginName) == 0)
						return plugin;
				}

				return NULL;
			}

			void installPlugin(mrk::Plugin *plugin) { this->plugins.push_back(plugin); }

			StoredArray<entitlement_callback_t>& getEntitlementCallbacks() { return entitlementCallbacks; }

			StoredArray<binaryload_callback_t>& getBinaryLoadCallbacks() { return binaryLoadCallbacks; }

			StoredArray<kextload_callback_t>& getKextLoadCallbacks() { return kextLoadCallbacks; }

			void registerCallbacks();

			void registerEntitlementCallback(void *user, entitlement_callback_t callback);

			void registerBinaryLoadCallback(void *user, binaryload_callback_t callback);

			void registerKextLoadCallback(void *user, kextload_callback_t callback);

			void onEntitlementRequest(task_t task, const char *entitlement, void *original);

			void onProcLoad(task_t task, const char *path, Size len);

			void onKextLoad(void *kext, xnu::KmodInfo *kmod);

			xnu::KmodInfo* findKmodInfo(const char *kextname);

			void* findOSKextByIdentifier(const char *kextidentifier);

		private:
			Arch::Architecture *architecture;

			xnu::Kernel *kernel;

			mrk::KernelPatcher *kernelPatcher;

			enum Arch::Architectures platformArchitecture;

			bool waitingForAlreadyLoadedKexts;

			std::vector<xnu::Kext*> kexts;

			xnu::KmodInfo **kextKmods;

			std::vector<mrk::Plugin*> plugins;

			StoredArray<entitlement_callback_t> entitlementCallbacks;

			StoredArray<binaryload_callback_t> binaryLoadCallbacks;

			StoredArray<kextload_callback_t> kextLoadCallbacks;
	};
}
