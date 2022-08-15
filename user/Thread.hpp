#ifndef __THREAD_HPP_
#define __THREAD_HPP_

#include <stdint.h>
#include <string.h>

#include <sys/types.h>
#include <mach/mach_types.h>

class Thread
{
	public:
		Thread() { }

		~Thread() { }

		Task* getTask() { return task; }

		mach_port_t getPortHandle() { return handle; }

		mach_vm_address_t getThread() { return thread; }
		mach_vm_address_t  getUThread() { return uthread; }

	private:
		Task *task;

		mach_port_t handle;

		mach_port_t thread;
		mach_port_t uthread;
};

#endif