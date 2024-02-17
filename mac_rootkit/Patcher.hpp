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

#include <mach/kmod.h>

#include "vector.hpp"
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

			std::vector<Hook*>& getHooks() { return hooks; }

			mrk::Hook* hookForFunction(mach_vm_address_t address);

			mrk::Hook* breakpointForAddress(mach_vm_address_t address);

			bool isFunctionHooked(mach_vm_address_t address);

			bool isBreakpointAtInstruction(mach_vm_address_t address);

			void installHook(mrk::Hook *hook, mach_vm_address_t hooked);

			void removeHook(mrk::Hook *hook);

		private:
			std::vector<mrk::Hook*> hooks;
	};

}
