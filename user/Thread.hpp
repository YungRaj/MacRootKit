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
