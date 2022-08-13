#ifndef __USERPATCHER_HPP_
#define __USERPATCHER_HPP_

#include "Patcher.hpp"

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

#endif