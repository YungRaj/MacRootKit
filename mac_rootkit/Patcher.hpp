#ifndef __PATCHER_HPP_
#define __PATCHER_HPP_

#include <mach/kmod.h>

#include "Array.hpp"
#include "Pair.hpp"

namespace mrk
{
	class Hook;
	
	class Patcher
	{
		public:
			Patcher();

			~Patcher();

			virtual void findAndReplace(void *data,
										size_t data_size,
										const void *find, size_t find_size,
										const void *replace, size_t replace_size);

			virtual void onKextLoad(void *kext, kmod_info_t *kmod);

			virtual void routeFunction(mrk::Hook *hook);

			std::Array<Hook*>* getHooks() { return &hooks; }

			mrk::Hook* hookForFunction(mach_vm_address_t address);

			mrk::Hook* breakpointForAddress(mach_vm_address_t address);

			bool isFunctionHooked(mach_vm_address_t address);

			bool isBreakpointAtInstruction(mach_vm_address_t address);

			void installHook(mrk::Hook *hook, mach_vm_address_t hooked);

			void removeHook(mrk::Hook *hook);

		private:
			std::Array<mrk::Hook*> hooks;
	};

}

#endif