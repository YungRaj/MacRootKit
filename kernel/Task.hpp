#ifndef __TASK_HPP_
#define __TASK_HPP_

#include "Process.hpp"

#include "Disassembler.hpp"

#include "MachO.hpp"
#include "Segment.hpp"
#include "Section.hpp"

namespace xnu
{
	class Kernel;
}

class Disassembler;

typedef void* pmap_t;

typedef void* proc_t;

namespace xnu
{
	class Task
	{
		public:
			Task();

			Task(Kernel *kernel, mach_port_t task_port);
			Task(Kernel *kernel, task_t task);

			static mach_port_t getTaskPort(Kernel *kernel, int pid);

			static Task* getTaskByName(Kernel *kernel, char *name);

			static proc_t findProcByPid(Kernel *kernel, int pid);
			static proc_t findProcByName(Kernel *kernel, char *name);

			static task_t findTaskByPid(Kernel *kernel, int pid);
			static task_t findTaskByName(Kernel *kernel, char *name);

			task_t getTask() { return task; }

			vm_map_t getMap() { return map; }

			pmap_t getPmap() { return pmap; }

			proc_t getProc() { return proc; }

			Disassembler* getDisassembler() { return disassembler; }

			bsd::Process* getProcess() { return process; }

			int getPid() { return pid; }

			void setTask(task_t task) { this->task = task; }

			void setMap(vm_map_t map) { this->map = map; }

			void setPmap(pmap_t pmap) { this->pmap = pmap; }

			void setProc(proc_t proc) { this->proc = proc; }

			virtual mach_vm_address_t getBase() { return base; }

			virtual off_t getSlide() { return slide; }

			void setBase(mach_vm_address_t base) { this->base = base; }

			void setSlide(off_t slide) { this->slide = slide; }

			virtual uint64_t call(char *symbolname, uint64_t *arguments, size_t argCount);
			virtual uint64_t call(mach_vm_address_t func, uint64_t *arguments, size_t argCount);

			virtual mach_vm_address_t vmAllocate(size_t size);
			virtual mach_vm_address_t vmAllocate(size_t size, uint32_t flags, vm_prot_t prot);

			virtual void vmDeallocate(mach_vm_address_t address, size_t size);

			virtual bool vmProtect(mach_vm_address_t address, size_t size, vm_prot_t prot);

			virtual void* vmRemap(mach_vm_address_t address, size_t size);

			virtual uint64_t virtualToPhysical(mach_vm_address_t address);

			virtual bool read(mach_vm_address_t address, void *data, size_t size);

			virtual uint8_t read8(mach_vm_address_t address);
			virtual uint16_t read16(mach_vm_address_t address);
			virtual uint32_t read32(mach_vm_address_t address);
			virtual uint64_t read64(mach_vm_address_t address);

			virtual bool write(mach_vm_address_t address, void *data, size_t size);

			virtual void write8(mach_vm_address_t address, uint8_t value);
			virtual void write16(mach_vm_address_t address, uint16_t value);
			virtual void write32(mach_vm_address_t address, uint32_t value);
			virtual void write64(mach_vm_address_t address, uint64_t value);

			virtual char* readString(mach_vm_address_t address);

			virtual Symbol* getSymbolByName(char *symname);
			virtual Symbol* getSymbolByAddress(mach_vm_address_t address);

			virtual mach_vm_address_t getSymbolAddressByName(char *symbolname);

		protected:
			Kernel *kernel;

			Disassembler *disassembler;

			mach_port_t task_port;

			bsd::Process *process;

			task_t task;
			proc_t proc;

			vm_map_t map;
			pmap_t pmap;

			mach_vm_address_t base;

			off_t slide;

			mach_vm_address_t dyld = 0;
			mach_vm_address_t dyld_shared_cache = 0;

			int pid;
	};

}

#endif