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

#include "UserPatcher.hpp"

#include "Hook.hpp"
#include "Payload.hpp"

UserPatcher::~UserPatcher()
{

}

void UserPatcher::findAndReplace(void *data, 
					size_t dataSize,
					const void *find,
					size_t findSize,
					const void* replace,
					size_t replaceSize)
{

}

void UserPatcher::routeFunction(Hook *hook)
{

}

void UserPatcher::onKextLoad(void *kext, kmod_info_t *kmod)
{

}

void UserPatcher::onExec(char *name, int pid, mach_port_t port, mach_vm_address_t task, mach_vm_address_t proc)
{

}

mach_vm_address_t injectPayload(mach_vm_address_t address, Payload *payload)
{
	return 0;
}

mach_vm_address_t injectSegment(mach_vm_address_t address, Payload *payload)
{
	return 0;
}