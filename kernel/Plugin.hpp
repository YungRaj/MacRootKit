#ifndef __PLUGIN_HPP_
#define __PLUGIN_HPP_

extern "C"
{
	#include <sys/types.h>
}

#include <IOKit/IOService.h>

#include "API.h"
#include "APIUtil.hpp"

#include "Array.hpp"

namespace xnu
{
	class Kernel;
	class Kext;
};

namespace mrk
{
	struct Plugin
	{
		public:
			explicit Plugin(IOService *service,
							char *product, size_t version, uint32_t runmode,
							const char **disableArg, size_t disableArgNum,
							const char **debugArg, size_t debugArgNum,
							const char **betaArg, size_t betaArgNum);

			explicit Plugin(char *product, size_t version, uint32_t runmode,
							const char **disableArg, size_t disableArgNum,
							const char **debugArg, size_t debugArgNum,
							const char **betaArg, size_t betaArgNum);

			size_t getVersion() { return version; }

			IOService* getService() { return service; }

			const char* getProduct() { return product; }

			bool isKextPlugin() { return service != NULL; }

			void addTarget(void *t)
			{
				union Target target;

				target.target = t;

				this->targets.add(target);
			}

			void addHook(xnu::Kernel *kernel, Hook *hook) { this->addTarget(kernel); this->hooks.add(hook); }

			void addHook(xnu::Kext *kext, Hook *hook) { this->addTarget(kext); this->hooks.add(hook); }

			void removeHook(Hook *hook) { this->hooks.remove(hook); }
			
			void (*pluginStart)();
			void (*pluginStop)();
		
		private:
			union Target
			{
				void *target;

				xnu::Kernel *kernel;
				xnu::Kext *kext;
			};

			std::Array<union Target> targets;

			IOService *service;

			const char *product;

			size_t version;
			
			uint32_t runmode;
			
			const char **disableArg;
			size_t disableArgNum;
			
			const char **debugArg;
			size_t debugArgNum;

			const char **betaArg;
			size_t betaArgNum;

			std::Array<Hook*> hooks;
	};
};

#endif