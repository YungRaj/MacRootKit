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

#include "Patcher.hpp"

namespace mrk
{
	class Hook;
	class Payload;

	class UserPatcher : Patcher
	{
		public:
			UserPatcher();

			~UserPatcher();

			virtual void findAndReplace(void *data,
										size_t data_size,
										const void *find, size_t find_size,
										const void *replace, size_t replace_size);

			virtual void routeFunction(Hook *hook);

			virtual void onExec(char *name, int pid, mach_port_t port, mach_vm_address_t task, mach_vm_address_t proc);

			virtual void onKextLoad(void *kext, kmod_info_t *kmod);

			mach_vm_address_t injectPayload(mach_vm_address_t address, Payload *payload);

			mach_vm_address_t injectSegment(mach_vm_address_t address, Payload *payload);

			size_t mapAddresses(const char *mapBuf);

		private:
	};
}
