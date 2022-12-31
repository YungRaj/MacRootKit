#ifndef __TASK_HPP_
#define __TASK_HPP_

#include <stdint.h>
#include <string.h>

#include <sys/types.h>
#include <mach/mach_types.h>

namespace xnu
{
	class Kernel;
};

using namespace xnu;

class Dyld;
class Process;

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

			Task(Kernel *kernel, int pid);

			Task(Kernel *kernel, char *name);
			
			Task(Kernel *kernel, mach_port_t task_port);

			~Task();

			Kernel* getKernel();

			int getPid() { return pid; }

			int findPid();

			mach_port_t getTaskPort() { return task_port; }

			mach_vm_address_t getTask() { return task; }

			mach_vm_address_t getProc() { return proc; }

			char* getName() { return name; }

			Dyld* getDyld() { return dyld; }

			Process* getProcess() { return process; }

			Disassembler* getDisassembler() { return disassembler; }

			static mach_port_t getTaskForPid(int pid);

			static Task* getTaskInfo(Kernel *kernel, char *task_name);

			static mach_vm_address_t findProcByPid(Kernel *kernel, int pid);
			static mach_vm_address_t findProcByName(Kernel *kernel, char *name);

			static mach_vm_address_t findTaskByPid(Kernel *kernel, int pid);
			static mach_vm_address_t findTaskByName(Kernel *kernel, char *name);

			static mach_vm_address_t getTaskFromProc(Kernel *kernel, mach_vm_address_t proc);

			static mach_vm_address_t findPort(Kernel *kernel, mach_vm_address_t task, mach_port_t port);

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
			Kernel *kernel;

			UserMachO *macho;

			Disassembler *disassembler;

			mach_port_t task_port;

			off_t slide;

			mach_vm_address_t task;
			mach_vm_address_t proc;

			mach_vm_address_t map;
			mach_vm_address_t pmap;

			char *name; 
			char *path;

			Process *process;

			int pid;

			mach_vm_address_t base;

			mach_vm_address_t dyld_base;
			mach_vm_address_t dyld_shared_cache;

			Dyld *dyld;
	};
};

#endif