#include "Payload.hpp"

#include "Hook.hpp"

using namespace mrk;
using namespace xnu;

Payload::Payload(Task *task, Hook *hook, vm_prot_t protection)
{
	this->task = task;
	this->hook = hook;
	this->prot = protection;
	this->current_offset = 0;
}

Payload::~Payload()
{
}

bool Payload::readBytes(uint8_t *bytes, size_t size)
{
	bool success;

	success = this->readBytes(this->current_offset, bytes, size);

	return success;
}

bool Payload::readBytes(off_t offset, uint8_t *bytes, size_t size)
{
	bool success;

	mach_vm_address_t address = this->address + offset;

	success = this->getTask()->read(address + offset, (void*) bytes, size);

	return success;
}

bool Payload::writeBytes(uint8_t *bytes, size_t size)
{
	bool success;

	success = this->writeBytes(this->current_offset, bytes, size);

	if(success)
		this->current_offset += size;

	return success;
}

bool Payload::writeBytes(off_t offset, uint8_t *bytes, size_t size)
{
	bool success;

	mach_vm_address_t address = this->address + offset;

	success = this->getTask()->write(address, (void*) bytes, size);

#ifdef __KERNEL__

	if(address >= (mach_vm_address_t) Kernel::getExecutableMemory() && address < (mach_vm_address_t) Kernel::getExecutableMemory() + Kernel::getExecutableMemorySize())
	{
		Kernel::setExecutableMemoryOffset(Kernel::getExecutableMemoryOffset() + size);
	}

#endif

	return success;
}

bool Payload::prepare()
{
	bool success;

	mach_vm_address_t trampoline;

	Task *task = this->getTask();

#if 0 // defined(__x86_64__) || (defined(__arm64__) && defined(__USER__))

	trampoline = task->vmAllocate(Payload::expectedSize, VM_FLAGS_ANYWHERE, VM_PROT_READ | VM_PROT_EXECUTE);

	if(!trampoline)
		return false;

/*#elif defined(__arm64__) && defined(__KERNEL__)*/
#else

	trampoline = Kernel::getExecutableMemory() + Kernel::getExecutableMemoryOffset();

#endif

	this->address = trampoline;

	return true;
}

void Payload::setWritable()
{
	this->task->vmProtect(this->address, Payload::expectedSize, VM_PROT_READ | VM_PROT_WRITE);
}

void Payload::setExecutable()
{
	this->task->vmProtect(this->address, Payload::expectedSize, VM_PROT_READ | VM_PROT_EXECUTE);
}

bool Payload::commit()
{

#if 0 // defined(__x86_64__) || (defined(__arm64__) && defined(__USER__))

	ml_set_interrupts_enabled(false);

	this->setExecutable();

	ml_set_interrupts_enabled(true);

#endif

	return true;
}
