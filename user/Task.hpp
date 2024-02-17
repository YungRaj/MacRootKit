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

#include <sys/types.h>
#include <mach/mach_types.h>

namespace xnu
{
	class Kernel;
};

namespace dyld
{
	class Dyld;
};

namespace bsd
{
	class Process;
};

class Disassembler;

class Symbol;
class Segment;
class Section;

class MachO;

namespace mrk
{
	class UserMachO;
};

using namespace mrk;

namespace xnu
{
	class Task
	{
		public:
			Task();

			explicit Task(xnu::Kernel *kernel, int pid);

			explicit Task(xnu::Kernel *kernel, char *name);
			
			explicit Task(xnu::Kernel *kernel, mach_port_t task_port);

			~Task();

			xnu::Kernel* getKernel();

			int getPid() { return pid; }

			int findPid();

			mach_port_t getTaskPort() { return task_port; }

			mach_vm_address_t getTask() { return task; }

			mach_vm_address_t getProc() { return proc; }

			char* getName() { return name; }

			dyld::Dyld* getDyld() { return dyld; }

			bsd::Process* getProcess() { return process; }

			Disassembler* getDisassembler() { return disassembler; }

			static mach_port_t getTaskForPid(int pid);

			static Task* getTaskInfo(Kernel *kernel, char *task_name);

			static mach_vm_address_t findProcByPid(xnu::Kernel *kernel, int pid);
			static mach_vm_address_t findProcByName(xnu::Kernel *kernel, char *name);

			static mach_vm_address_t findTaskByPid(xnu::Kernel *kernel, int pid);
			static mach_vm_address_t findTaskByName(xnu::Kernel *kernel, char *name);

			static mach_vm_address_t getTaskFromProc(xnu::Kernel *kernel, mach_vm_address_t proc);

			static mach_vm_address_t findPort(xnu::Kernel *kernel, mach_vm_address_t task, mach_port_t port);

			virtual mach_vm_address_t getBase();

			virtual off_t getSlide();

			virtual char* getTaskName();

			virtual uint64_t call(char *symbolname, uint64_t *arguments, size_t argCount);
			virtual uint64_t call(mach_vm_address_t func, uint64_t *arguments, size_t argCount);

			virtual mach_vm_address_t vmAllocate(size_t size);
			virtual mach_vm_address_t vmAllocate(size_t size, uint32_t flags, vm_prot_t prot);

			virtual void vmDeallocate(mach_vm_address_t address, size_t size);

			virtual bool vmProtect(mach_vm_address_t address, size_t size, vm_prot_t prot);

			virtual void* vmRemap(mach_vm_address_t address, size_t size);

			virtual uint64_t virtualToPhysical(mach_vm_address_t address);

			virtual bool read(mach_vm_address_t address, void *data, size_t size);
			virtual bool readUnsafe(mach_vm_address_t address, void *data, size_t size);

			virtual uint8_t read8(mach_vm_address_t address);
			virtual uint16_t read16(mach_vm_address_t address);
			virtual uint32_t read32(mach_vm_address_t address);
			virtual uint64_t read64(mach_vm_address_t address);

			virtual bool write(mach_vm_address_t address, void *data, size_t size);
			virtual bool writeUnsafe(mach_vm_address_t address, void *data, size_t size);

			virtual void write8(mach_vm_address_t address, uint8_t value);
			virtual void write16(mach_vm_address_t address, uint16_t value);
			virtual void write32(mach_vm_address_t address, uint32_t value);
			virtual void write64(mach_vm_address_t address, uint64_t value);

			virtual char* readString(mach_vm_address_t address);

			virtual Symbol* getSymbolByName(char *symname);
			virtual Symbol* getSymbolByAddress(mach_vm_address_t address);

			virtual mach_vm_address_t getSymbolAddressByName(char *symbolname);

			mach_vm_address_t getImageLoadedAt(char *image_name, char **image_path);

			virtual void printLoadedImages();

		protected:
			xnu::Kernel *kernel;

			mrk::UserMachO *macho;

			Disassembler *disassembler;

			mach_port_t task_port;

			off_t slide;

			mach_vm_address_t task;
			mach_vm_address_t proc;

			mach_vm_address_t map;
			mach_vm_address_t pmap;

			char *name; 
			char *path;

			bsd::Process *process;

			int pid;

			mach_vm_address_t base;

			mach_vm_address_t dyld_base;
			mach_vm_address_t dyld_shared_cache;

			dyld::Dyld *dyld;
	};
};
