#ifndef __PLUGIN_HPP_
#define __PLUGIN_HPP_

extern "C"
{
	#include <sys/types.h>
}

#include <IOKit/IOService.h>

#include "API.h"
#include "APIUtil.hpp"

#include "vector.hpp"

#include "Disassembler.hpp"

#include "Hook.hpp"

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

				this->targets.push_back(target);
			}

			void addHook(xnu::Kernel *kernel, Hook *hook) { this->addTarget(kernel); this->hooks.push_back(hook); }

			void addHook(xnu::Kext *kext, Hook *hook) { this->addTarget(kext); this->hooks.push_back(hook); }

			void removeHook(Hook *hook) { this->hooks.erase(std::remove(hooks.begin(), hooks.end(), hook), hooks.end()); }
			
			void (*pluginStart)();
			void (*pluginStop)();
		
		private:
			union Target
			{
				void *target;

				xnu::Kernel *kernel;
				xnu::Kext *kext;
			};

			std::vector<union Target> targets;

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

			std::vector<Hook*> hooks;
	};
};

#endif