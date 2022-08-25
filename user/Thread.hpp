#ifndef __THREAD_HPP_
#define __THREAD_HPP_

#include <stdint.h>
#include <string.h>

#include <pthread.h>

#include <sys/types.h>
#include <mach/mach_types.h>

class Thread
{
	public:
		Thread() { }

		~Thread() { }

		Task* getTask() { return task; }

		mach_port_t getPortHandle() { return handle; }

		pthread_t getPthread() { return pthread; }

		mach_vm_address_t getThread() { return thread; }
		mach_vm_address_t  getUThread() { return uthread; }

		arm_thread_state64_t* getThreadState() { return state; }

		void resume();

		void suspend();

		void terminate();

		void setThreadState(arm_thread_state64_t *thread_state);

		void getThreadState(arm_thread_state64_t *thread_state);

		void convertThreadState(Task *task, arm_thread_state64_t *thread_state);

		void setEntryPoint(mach_vm_address_t address);

		void setReturnAddress(mach_vm_address_t address);

		void pthreadCreateFromMachThread(mach_port_t thread);

	private:
		Task *task;

		mach_port_t handle;

		mach_vm_address_t thread;
		mach_vm_address_t uthread;

		mach_vm_address_t stack;
		mach_vm_address_t code;
		mach_vm_address_t data;

		pthread_t pthread;

		arm_thread_state64_t state;
};

#endif