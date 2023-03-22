#include <assert.h>

#include <mach/mach.h>

#include "Log.hpp"
#include "Kernel.hpp"
#include "Disassembler.hpp"

#include "Task.hpp"
#include "Dyld.hpp"

using namespace xnu;

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
	this->disassembler = new Disassembler(this);
}

Task::Task(Kernel *kernel, int pid)
{
	this->kernel = kernel;
	this->pid = pid;
	this->task_port = Task::getTaskForPid(pid);
	this->proc = Task::findProcByPid(kernel, pid);
	this->task = Task::getTaskFromProc(kernel, this->proc);
	this->name = this->getTaskName();
	this->dyld = new dyld::Dyld(kernel, this);
	this->macho = new mrk::UserMachO();
	// this->macho->withTask(this);
}

Task::Task(Kernel *kernel, char *name)
{
	this->kernel = kernel;
	this->name = name;
	this->proc = Task::findProcByName(kernel, name);
	this->task = Task::getTaskFromProc(kernel, this->proc);
	this->pid = this->findPid();
	this->task_port = Task::getTaskForPid(pid);
	this->dyld = new dyld::Dyld(kernel, this);
	this->macho = new mrk::UserMachO();

}
		
Task::Task(Kernel *kernel, mach_port_t task_port)
{
	this->kernel = kernel;
	this->task_port = task_port;
}

Task::~Task()
{
	
}

mach_port_t Task::getTaskForPid(int pid)
{
	kern_return_t ret;

	mach_port_t task;

	ret = task_for_pid(mach_task_self(), pid, &task);

	if(ret != KERN_SUCCESS)
	{
		MAC_RK_LOG("task_for_pid() failed! ret = %d\n", ret);
	}

	assert(task != MACH_PORT_NULL);
	assert(ret == KERN_SUCCESS);

	return task;
}

int Task::findPid()
{
	uint64_t arguments[] = { this->proc };

	int pid = kernel->call("_proc_pid", arguments, 1);

	return pid;
}

Task* Task::getTaskInfo(Kernel *kernel, char *task_name)
{
	return NULL;
}

mach_vm_address_t Task::findProcByPid(Kernel *kernel, int pid)
{
	mach_vm_address_t current_proc;

	int current_pid;

	current_proc = (mach_vm_address_t) kernel->read64(kernel->getSymbolAddressByName("_kernproc"));

	current_proc = (mach_vm_address_t) kernel->read64(current_proc + 0x8);

	while(current_proc)
	{
		uint64_t arguments[] = { current_proc };

		current_pid = kernel->call("_proc_pid", arguments, 1);

		if(current_pid == pid)
		{
			return current_proc;
		}

		if(current_pid == 0)
			break;

		current_proc = (mach_vm_address_t) kernel->read64(current_proc + 0x8);
	}

	current_proc = get_proc_for_pid(pid);

	if(current_proc)
	{
		return current_proc;
	}

	MAC_RK_LOG("MacRK::could not find proc for pid = %d\n", pid);

	assert(false);

	return 0;
}

mach_vm_address_t Task::findProcByName(Kernel *kernel, char *name)
{
	mach_vm_address_t current_proc;

	current_proc = kernel->read64(kernel->getSymbolAddressByName("_kernproc"));

	current_proc = kernel->read64(current_proc + 0x8);

	while(current_proc)
	{
		uint64_t arguments[] = { current_proc };

		mach_vm_address_t proc_name;

		proc_name = kernel->call("_proc_name", arguments, 1);

		if(EndsWith(name, kernel->readString(proc_name)) == 0)
		{
			return current_proc;
		}

		current_proc = kernel->read64(current_proc + 0x8);
	}

	current_proc = get_proc_by_name(name);

	if(current_proc)
	{
		return current_proc;
	}

	MAC_RK_LOG("MacRK::could not find proc for name = %s\n", name);

	assert(false);

	return 0;
}

mach_vm_address_t Task::findTaskByPid(Kernel *kernel, int pid)
{
	mach_vm_address_t task;
	mach_vm_address_t proc;

	proc = Task::findProcByPid(kernel, pid);

	if(proc)
	{
		uint64_t arguments[] = { proc };

		task = kernel->call("_proc_task", arguments, 1);

		return task;
	}

	task = get_task_for_pid(pid);

	if(task)
	{
		return task;
	}

	MAC_RK_LOG("MacPE::could not find task by pid = %d\n", pid);

	assert(false);

	return 0;
}

mach_vm_address_t Task::findTaskByName(Kernel *kernel, char *name)
{
	mach_vm_address_t task;
	mach_vm_address_t proc;

	proc = Task::findProcByName(kernel, name);

	if(proc)
	{
		uint64_t arguments[] = { proc };

		task = kernel->call("_proc_task", arguments, 1);

		return task;
	}

	task = get_task_by_name(name);

	if(task)
	{
		return task;
	}

	MAC_RK_LOG("MacPE::could not find task by name = %s\n", name);

	assert(false);

	return 0;
}

mach_vm_address_t Task::getTaskFromProc(Kernel *kernel, mach_vm_address_t proc)
{
	mach_vm_address_t task;

	if(proc)
	{
		uint64_t arguments[] = { proc };

		task = kernel->call("_proc_task", arguments, 1);

		return task;
	}

	return 0;
}

char* Task::getTaskName()
{
	char *task_name;

	mach_vm_address_t name;

	uint64_t kalloc_args[] = { 256 };

	uint64_t buffer = this->kernel->call("_kalloc", kalloc_args, 1);

	uint64_t proc_name_args[] = { static_cast<uint64_t>(this->pid) , buffer, 256 };

	name = this->kernel->call("_proc_name", proc_name_args, 3);

	task_name = this->kernel->readString(buffer);

	uint64_t kfree_args[] = { buffer, 256 };

	this->kernel->call("_kfree", kfree_args, 2);

	return task_name;
}

mach_vm_address_t Task::findPort(Kernel *kernel, mach_vm_address_t task, mach_port_t port)
{
	return 0;
}

mach_vm_address_t Task::getBase()
{
	return 0;
}

off_t Task::getSlide()
{
	return 0;
}

uint64_t Task::call(char *symbolname, uint64_t *arguments, size_t argCount)
{
	// return task_call(task_port, symaddr, arguments, argcount);
	return 0;
}

uint64_t Task::call(mach_vm_address_t func, uint64_t *arguments, size_t argCount)
{
	return task_call(task_port, func, arguments, argCount);
}

mach_vm_address_t Task::vmAllocate(size_t size)
{
	kern_return_t ret;

	mach_vm_address_t address;

	ret = vm_allocate(this->task_port, (vm_address_t*) &address, size, VM_PROT_DEFAULT);

	if(ret == KERN_SUCCESS)
	{
		return address;
	}

	MAC_RK_LOG("mach_vm_allocate() failed!\n");

	assert(ret == KERN_SUCCESS);

	return task_vm_allocate(task_port, size);
}

mach_vm_address_t Task::vmAllocate(size_t size, uint32_t flags, vm_prot_t prot)
{
	kern_return_t ret;

	mach_vm_address_t address;

	ret = vm_allocate(this->task_port, (vm_address_t*) &address, size, prot);

	if(ret == KERN_SUCCESS)
	{
		return address;
	}

	MAC_RK_LOG("mach_vm_allocate() failed!\n");

	assert(ret == KERN_SUCCESS);

	return task_vm_allocate(task_port, size);
}

void Task::vmDeallocate(mach_vm_address_t address, size_t size)
{
	kern_return_t ret;

	ret = vm_deallocate(this->task_port, address, size);

	if(ret != KERN_SUCCESS)
	{
		MAC_RK_LOG("mach_vm_deallocate() failed!\n");
	}

	assert(ret == KERN_SUCCESS);

	if(ret != KERN_SUCCESS)
	{
		task_vm_deallocate(task_port, address, size);
	}
}

bool Task::vmProtect(mach_vm_address_t address, size_t size, vm_prot_t prot)
{
	kern_return_t ret;

	ret = vm_protect(this->task_port, address, size, FALSE, prot);

	if(ret == KERN_SUCCESS)
	{
		return true;
	}

	MAC_RK_LOG("mach_vm_protect() failed!\n");

	assert(ret == KERN_SUCCESS);

	return task_vm_protect(task_port, address, size, prot);
}

void* Task::vmRemap(mach_vm_address_t address, size_t size)
{
	kern_return_t ret;

	mach_vm_address_t remap = 0;

	// ret = vm_remap(this->task_port, address, size, &remap);

	if(ret == KERN_SUCCESS)
	{
		return reinterpret_cast<void*>(remap);
	}

	MAC_RK_LOG("mach_vm_remap() failed!\n");

	assert(ret == KERN_SUCCESS);

	return NULL;
}

uint64_t Task::virtualToPhysical(mach_vm_address_t address)
{
	return 0;
}

bool Task::read(mach_vm_address_t address, void *data, size_t size)
{
	kern_return_t ret;

	mach_msg_type_number_t dataCount = size;

	ret = vm_read_overwrite(this->task_port, address, size, (mach_vm_address_t) data, (vm_size_t*) &dataCount);

	if(ret == KERN_SUCCESS)
	{
		return true;
	}

	MAC_RK_LOG("mach_vm_read() failed! %d\n", ret);

	assert(ret == KERN_SUCCESS);

	return task_vm_read(task_port, address, data, size);
}

bool Task::readUnsafe(mach_vm_address_t address, void *data, size_t size)
{
	return false;
}

uint8_t Task::read8(mach_vm_address_t address)
{
	bool success;

	uint8_t read;

	success = this->read(address, reinterpret_cast<void*>(&read), sizeof(uint8_t));

	if(!success)
	{
		MAC_RK_LOG("Task::read8() failed!\n");
	}

	assert(success);

	return read;
}

uint16_t Task::read16(mach_vm_address_t address)
{
	bool success;

	uint16_t read;

	success = this->read(address, reinterpret_cast<void*>(&read), sizeof(uint16_t));

	if(!success)
	{
		MAC_RK_LOG("Task::read16() failed!\n");
	}

	assert(success);

	return read;
}

uint32_t Task::read32(mach_vm_address_t address)
{
	bool success;

	uint32_t read;

	success = this->read(address, reinterpret_cast<void*>(&read), sizeof(uint32_t));

	if(!success)
	{
		MAC_RK_LOG("Task::read32() failed!\n");
	}

	assert(success);

	return read;
}

uint64_t Task::read64(mach_vm_address_t address)
{
	bool success;

	uint64_t read;

	success = this->read(address, reinterpret_cast<void*>(&read), sizeof(uint64_t));

	if(!success)
	{
		MAC_RK_LOG("Task::read64() failed!\n");
	}

	assert(success);

	return read;
}

bool Task::write(mach_vm_address_t address, void *data, size_t size)
{
	return task_vm_write(task_port, address, data, size);
}

bool Task::writeUnsafe(mach_vm_address_t address, void *data, size_t size)
{
	return false;
}

void Task::write8(mach_vm_address_t address, uint8_t value)
{
	bool success;

	success = this->write(address, reinterpret_cast<void*>(&value), sizeof(uint8_t));

	if(!success)
	{
		MAC_RK_LOG("Task::write8() failed!\n");
	}

	assert(success);
}

void Task::write16(mach_vm_address_t address, uint16_t value)
{
	bool success;

	success = this->write(address, reinterpret_cast<void*>(&value), sizeof(uint16_t));

	if(!success)
	{
		MAC_RK_LOG("Task::write16() failed!\n");
	}

	assert(success);
}

void Task::write32(mach_vm_address_t address, uint32_t value)
{
	bool success;

	success = this->write(address, reinterpret_cast<void*>(&value), sizeof(uint32_t));

	if(!success)
	{
		MAC_RK_LOG("Task::write32() failed!\n");
	}

	assert(success);
}

void Task::write64(mach_vm_address_t address, uint64_t value)
{
	bool success;

	success = this->write(address, reinterpret_cast<void*>(&value), sizeof(uint64_t));

	if(!success)
	{
		MAC_RK_LOG("Task::write64() failed!\n");
	}

	assert(success);
}

char* Task::readString(mach_vm_address_t address)
{
	char *string;

	uint8_t value;

	size_t size = 0;

	do
	{
		this->read(address + size++, &value, sizeof(value));
	} 
	while(value);

	assert(size > 0);

	string = (char*) malloc(size + 1);

	this->read(address, string, size + 1);

	return string;
}

Symbol* Task::getSymbolByName(char *symname)
{
	return macho->getSymbolByName(symname);
}

Symbol* Task::getSymbolByAddress(mach_vm_address_t address)
{
	return macho->getSymbolByAddress(address);
}

mach_vm_address_t Task::getSymbolAddressByName(char *symbolname)
{
	Symbol *symbol = this->macho->getSymbolByName(symbolname);

	if(!symbol)
		return 0;

	return symbol->getAddress();
}

mach_vm_address_t Task::getImageLoadedAt(char *image_name, char **image_path)
{
	mach_vm_address_t image = this->dyld->getImageLoadedAt(image_name, image_path);

	if(!image)
	{
		MAC_RK_LOG("Locating image %s failed!\n", image_name);
	}

	assert(image != 0);

	return image;
}

void Task::printLoadedImages()
{
	
}
