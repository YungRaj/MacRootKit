#ifndef __THREAD_HPP_
#define __THREAD_HPP_

#include <stdint.h>
#include <string.h>

#include <pthread.h>

#include <sys/types.h>
#include <mach/mach_types.h>

#include <Arch.hpp>

namespace xnu
{
	class Thread
	{
		public:
			Thread() { }

			~Thread() { }

			thread_t getThread() { return thread; }

			pthread_t getPthread() { return pthread; }

			Task* getTask() { return task; }

			mach_port_t getPortHandle() { return handle; }

			mach_vm_address_t getThread() { return thread; }
			mach_vm_address_t  getUThread() { return uthread; }

			union ThreadState* getThreadState() { return &state; }

			void resume();

			void suspend();

			void terminate();

			void setThreadState(union ThreadState *thread_state);

			void getThreadState(union ThreadState *thread_state);

			void convertThreadState(Task *task, union ThreadState *thread_state);

			void setEntryPoint(mach_vm_address_t address);

			void setReturnAddress(mach_vm_address_t address);

			void createPosixThreadFromMachThread(thread_t thread);

		private:
			xnu::Task *task;

			pthread_t pthread;

			thread_t thread;

			mach_port_t handle;

			mach_vm_address_t thread;
			mach_vm_address_t uthread;

			mach_vm_address_t stack;
			mach_vm_address_t code;
			mach_vm_address_t data;

			union ThreadState state;
	};
}

#endif