#include "Task.hpp"
#include "Kernel.hpp"
#include "PAC.hpp"

#include "Log.hpp"

#include <mach/mach_types.h>
#include <IOKit/IOLib.h>

extern "C"
{
	#include "kern.h"
}

static int EndsWith(const char *str, const char *suffix)
{
	if (!str || !suffix)
		return 0;
	
	size_t lenstr = strlen(str);
	size_t lensuffix = strlen(suffix);
	
	if (lensuffix >  lenstr)
		return 0;
	return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

Task::Task()
{

}

Task::Task(Kernel *kernel, mach_port_t task_port)
{
	this->task_port = task_port;
	this->kernel = kernel;

	typedef vm_offset_t ipc_kobject_t;

	ipc_port_t port = reinterpret_cast<ipc_port_t>(task_port);

	uint8_t *port_buf = reinterpret_cast<uint8_t*>(port);

	this->task = (task_t) *(uint64_t*) (port_buf + 0x68);

	typedef proc_t (*get_bsdtask_info) (task_t task);
	proc_t (*_get_bsdtask_info) (task_t task);

	_get_bsdtask_info = reinterpret_cast<get_bsdtask_info> (kernel->getSymbolAddressByName("_get_bsdtask_info"));

	this->proc = _get_bsdtask_info(this->task);

	typedef vm_map_t (*get_task_map) (task_t task);
	vm_map_t (*_get_task_map)(task_t);

	_get_task_map = reinterpret_cast<get_task_map> (kernel->getSymbolAddressByName("_get_task_map"));

	this->map = _get_task_map(this->task);

	typedef pmap_t (*get_task_pmap) (task_t task);
	pmap_t (*_get_task_pmap)(task_t);

	_get_task_pmap = reinterpret_cast<get_task_pmap> (kernel->getSymbolAddressByName("_get_task_pmap"));

	this->pmap = _get_task_pmap(this->task);

	this->disassembler = new Disassembler(this);
}

Task::Task(Kernel *kernel, task_t task)
{
	this->task = task;

	typedef proc_t (*get_bsdtask_info) (task_t task);
	proc_t (*_get_bsdtask_info) (task_t task);

	_get_bsdtask_info = reinterpret_cast<get_bsdtask_info> (kernel->getSymbolAddressByName("_get_bsdtask_info"));

	this->proc = _get_bsdtask_info(this->task);

	typedef vm_map_t (*get_task_map) (task_t task);
	vm_map_t (*_get_task_map)(task_t);

	_get_task_map = reinterpret_cast<get_task_map> (kernel->getSymbolAddressByName("_get_task_map"));

	this->map = _get_task_map(this->task);

	typedef pmap_t (*get_task_pmap) (task_t task);
	pmap_t (*_get_task_pmap)(task_t);

	_get_task_pmap = reinterpret_cast<get_task_pmap> (kernel->getSymbolAddressByName("_get_task_pmap"));

	this->pmap = _get_task_pmap(this->task);

	this->disassembler = new Disassembler(this);
}

mach_port_t Task::getTaskPort(Kernel *kernel, int pid)
{
	char buffer[128];

	proc_t proc = Task::findProcByPid(kernel, pid);

	snprintf(buffer, 128, "0x%llx", (mach_vm_address_t) proc);

	MAC_RK_LOG("MacPE::Task::getTaskPort() proc = %s\n", buffer);

	typedef task_t (*proc_task) (proc_t proc);
	task_t (*_proc_task) (proc_t proc);

	_proc_task = reinterpret_cast<proc_task>(kernel->getSymbolAddressByName("_proc_task"));

	task_t task = _proc_task(proc);

	snprintf(buffer, 128, "0x%llx", (mach_vm_address_t) task);

	MAC_RK_LOG("MacPE::Task::getTaskPort() task = %s\n", buffer);

	typedef ipc_port_t (*convert_task_to_port)(task_t task);
	ipc_port_t (*_convert_task_to_port) (task_t task);

	_convert_task_to_port = reinterpret_cast<convert_task_to_port> (kernel->getSymbolAddressByName("_convert_task_to_port"));

	ipc_port_t port = _convert_task_to_port(task);

	snprintf(buffer, 128, "0x%llx", (mach_vm_address_t) port);

	MAC_RK_LOG("MacPE::Task::getTaskPort() port = %s\n", buffer);

	if(!port) return NULL;

	return (mach_port_t) port;
}

Task* Task::getTaskByName(Kernel *kernel, char *name)
{
	Task *task_;

	char buffer[128];

	proc_t proc = Task::findProcByName(kernel, name);

	snprintf(buffer, 128, "0x%llx", (mach_vm_address_t) proc);

	MAC_RK_LOG("MacPE::Task::getTaskByName() proc = %s\n", buffer);

	typedef task_t (*proc_task) (proc_t proc);
	task_t (*_proc_task) (proc_t proc);

	_proc_task = reinterpret_cast<proc_task>(kernel->getSymbolAddressByName("_proc_task"));

	task_t task = _proc_task(proc);

	snprintf(buffer, 128, "0x%llx", (mach_vm_address_t) task);

	MAC_RK_LOG("MacPE::Task::getTaskByName() task = %s\n", buffer);

	typedef ipc_port_t (*convert_task_to_port)(task_t task);
	ipc_port_t (*_convert_task_to_port) (task_t task);

	_convert_task_to_port = reinterpret_cast<convert_task_to_port> (kernel->getSymbolAddressByName("_convert_task_to_port"));

	ipc_port_t port = _convert_task_to_port(task);

	snprintf(buffer, 128, "0x%llx", (mach_vm_address_t) port);

	MAC_RK_LOG("MacPE::Task::getTaskByName() port = %s\n", buffer);

	if(!port) return NULL;

	task_ = new Task(kernel, port);

	return task_;
}

proc_t Task::findProcByPid(Kernel *kernel, int pid)
{
	proc_t current_proc;

	int current_pid;

	char buffer[128];

	current_proc = *reinterpret_cast<proc_t*> (kernel->getSymbolAddressByName("_kernproc"));

	current_proc = (proc_t) *(uint64_t*) ((uint8_t*) current_proc + 0x8);

	while(current_proc)
	{
		typedef int (*proc_pid) (proc_t proc);
		int (*_proc_pid) (proc_t proc);

		_proc_pid = reinterpret_cast<proc_pid> (PACSignPointerWithAKey(kernel->getSymbolAddressByName("_proc_pid")));

		current_pid = _proc_pid(current_proc);

		snprintf(buffer, 128, "0x%llx", (mach_vm_address_t) current_proc);

		MAC_RK_LOG("MacPE::proc = %s pid = %d\n", buffer, current_pid);

		if(current_pid == pid)
		{
			return current_proc;
		}
		
		if(current_pid == 0)
			break;

		current_proc = (proc_t) *(uint64_t*) ((uint8_t*) current_proc + 0x8);
	}

	return NULL;
}

proc_t Task::findProcByName(Kernel *kernel, char *name)
{
	proc_t current_proc;

	current_proc = *(proc_t*) reinterpret_cast<proc_t*> (kernel->getSymbolAddressByName("_kernproc"));

	current_proc = (proc_t) *(uint64_t*) ((uint8_t*) current_proc + 0x8);

	while(current_proc)
	{
		char *current_name;

		int current_pid;

		uint64_t buffer;

		typedef int (*proc_pid) (proc_t proc);
		int (*_proc_pid) (proc_t proc);

		typedef void* (*kalloc) (size_t size);
		void* (*_kalloc) (size_t size);

		typedef void (*kfree) (void *p, size_t s);
		void (*_kfree) (void *p, size_t);

		typedef void (*proc_name) (int pid, char *name, size_t size);
		void (*_proc_name) (int pid, char *name, size_t size);

		_proc_pid = reinterpret_cast<proc_pid> (PACSignPointerWithAKey(kernel->getSymbolAddressByName("_proc_pid")));

		current_pid = _proc_pid(current_proc);

		_kalloc = reinterpret_cast<kalloc>(PACSignPointerWithAKey(kernel->getSymbolAddressByName("_kalloc")));

		buffer = (uint64_t) _kalloc(256);

		_proc_name = reinterpret_cast<proc_name>(PACSignPointerWithAKey(kernel->getSymbolAddressByName("_proc_name")));

		_proc_name(current_pid, (char*) buffer, 256);

		current_name = reinterpret_cast<char*>(buffer);

		char pointer[128];

		snprintf(pointer, 128, "0x%llx", (mach_vm_address_t) current_proc);

		MAC_RK_LOG("MacRK::proc = %s name = %s\n", pointer, current_name);

		if(EndsWith(current_name, name))
		{
			return current_proc;
		}

		_kfree = reinterpret_cast<kfree>(PACSignPointerWithAKey(kernel->getSymbolAddressByName("_kfree")));

		_kfree(reinterpret_cast<void*>(buffer), 256);

		current_proc = (proc_t) *(uint64_t*) ((uint8_t*) current_proc + 0x8);
	}

	return NULL;
}

task_t Task::findTaskByPid(Kernel *kernel, int pid)
{
	task_t task;
	proc_t proc;

	proc = Task::findProcByPid(kernel, pid);

	if(proc != NULL)
	{
		typedef task_t (*proc_task) (proc_t proc);

		task_t (*_proc_task) (proc_t proc);

		_proc_task = reinterpret_cast<proc_task> (PACSignPointerWithAKey(kernel->getSymbolAddressByName("_proc_task")));

		task = _proc_task(proc);

		return task;
	}

	return NULL;
}

task_t Task::findTaskByName(Kernel *kernel, char *name)
{
	task_t task;
	proc_t proc;

	proc = Task::findProcByName(kernel, name);

	if(proc != NULL)
	{
		typedef task_t (*proc_task) (proc_t proc);

		task_t (*_proc_task) (proc_t proc);

		_proc_task = reinterpret_cast<proc_task> (PACSignPointerWithAKey(kernel->getSymbolAddressByName("_proc_task")));

		task = _proc_task(proc);

		return task;
	}

	return NULL;
}

uint64_t Task::call(char *symbolname, uint64_t *arguments, size_t argCount)
{
	return 0;
}

uint64_t Task::call(mach_vm_address_t func, uint64_t *arguments, size_t argCount)
{
	return 0;
}

mach_vm_address_t Task::vmAllocate(size_t size)
{
	return task_vm_allocate(this->getMap(), size);
}

#define VM_KERN_MEMORY_KEXT            6

#define VM_INHERIT_SHARE               ((vm_inherit_t) 0)      /* shared with child */
#define VM_INHERIT_COPY                ((vm_inherit_t) 1)      /* copy into child */
#define VM_INHERIT_NONE                ((vm_inherit_t) 2)      /* absent from child */
#define VM_INHERIT_DONATE_COPY         ((vm_inherit_t) 3)      /* copy and delete */

#define VM_INHERIT_DEFAULT             VM_INHERIT_COPY

mach_vm_address_t Task::vmAllocate(size_t size, uint32_t flags, vm_prot_t prot)
{
	kern_return_t ret;

	mach_vm_address_t address = 0;

	vm_map_t map = this->getMap();

	uint64_t vmEnterArgs[13] = { reinterpret_cast<uint64_t>(map), (uint64_t) &address, size, 0, flags, VM_KERN_MEMORY_KEXT, 0, 0, FALSE, (uint64_t) prot, (uint64_t) VM_INHERIT_DEFAULT };

	ret = static_cast<kern_return_t>(this->call("_vm_map_enter", vmEnterArgs, 13));

	if(ret != KERN_SUCCESS)
	{
		address = 0;
	}

	return address;
}

void Task::vmDeallocate(mach_vm_address_t address, size_t size)
{
	task_vm_deallocate(this->getMap(), address, size);
}

bool Task::vmProtect(mach_vm_address_t address, size_t size, vm_prot_t prot)
{
	return task_vm_protect(this->getMap(), address, size, prot);
}

void* Task::vmRemap(mach_vm_address_t address, size_t size)
{
	return task_vm_remap(this->getMap(), address, size);
}

uint64_t Task::virtualToPhysical(mach_vm_address_t address)
{
	return 0;
}

bool Task::read(mach_vm_address_t address, void *data, size_t size)
{
	return task_vm_read(this->getMap(), address, data, size);
}

uint8_t Task::read8(mach_vm_address_t address)
{
	uint8_t value;

	bool success = task_vm_read(this->getMap(), address, reinterpret_cast<void*>(&value), sizeof(value));

	if(!success)
		return 0;

	return value;
}

uint16_t Task::read16(mach_vm_address_t address)
{
	uint16_t value;

	bool success = task_vm_read(this->getMap(), address, reinterpret_cast<void*>(&value), sizeof(value));

	if(!success)
		return 0;

	return value;
}

uint32_t Task::read32(mach_vm_address_t address)
{
	uint32_t value;

	bool success = task_vm_read(this->getMap(), address, reinterpret_cast<void*>(&value), sizeof(value));

	if(!success)
		return 0;

	return value;
}

uint64_t Task::read64(mach_vm_address_t address)
{
	uint64_t value;

	bool success = task_vm_read(this->getMap(), address, reinterpret_cast<void*>(&value), sizeof(value));

	if(!success)
		return 0;

	return value;
}

bool Task::write(mach_vm_address_t address, void *data, size_t size)
{
	return task_vm_write(this->getMap(), address, data, size);
}

void Task::write8(mach_vm_address_t address, uint8_t value)
{
	task_vm_write(this->getMap(), address, reinterpret_cast<void*>(&value), sizeof(value));
}

void Task::write16(mach_vm_address_t address, uint16_t value)
{
	task_vm_write(this->getMap(), address, reinterpret_cast<void*>(&value), sizeof(value));
}

void Task::write32(mach_vm_address_t address, uint32_t value)
{
	task_vm_write(this->getMap(), address, reinterpret_cast<void*>(&value), sizeof(value));
}

void Task::write64(mach_vm_address_t address, uint64_t value)
{
	task_vm_write(this->getMap(), address, reinterpret_cast<void*>(&value), sizeof(value));
}

char* Task::readString(mach_vm_address_t address)
{
	return NULL;
}

Symbol* Task::getSymbolByName(char *symname)
{
	return NULL;
}

Symbol* Task::getSymbolByAddress(mach_vm_address_t address)
{
	return NULL;
}

mach_vm_address_t Task::getSymbolAddressByName(char *symbolname)
{
	return (mach_vm_address_t) 0;
}