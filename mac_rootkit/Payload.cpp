#include "Payload.hpp"

#include "Hook.hpp"

using namespace mrk;
using namespace xnu;

Payload::Payload(Task *task, Hook *hook, xnu::Mach::VmProtection protection) : task(task), hook(hook), prot(protection), current_offset(0)
{
	
}

Payload::~Payload()
{
}

bool Payload::readBytes(UInt8 *bytes, Size size)
{
	bool success;

	success = this->readBytes(this->current_offset, bytes, size);

	return success;
}

bool Payload::readBytes(Offset offset, UInt8 *bytes, Size size)
{
	bool success;

	xnu::Mach::VmAddress address = this->address + offset;

	success = this->getTask()->read(address + offset, (void*) bytes, size);

	return success;
}

bool Payload::writeBytes(UInt8 *bytes, Size size)
{
	bool success;

	success = this->writeBytes(this->current_offset, bytes, size);

	if(success)
		this->current_offset += size;

	return success;
}

bool Payload::writeBytes(Offset offset, UInt8 *bytes, Size size)
{
	bool success;

	xnu::Mach::VmAddress address = this->address + offset;

	success = this->getTask()->write(address, (void*) bytes, size);

#ifdef __KERNEL__

	if(address >= (xnu::Mach::VmAddress) Kernel::getExecutableMemory() && address < (xnu::Mach::VmAddress) Kernel::getExecutableMemory() + Kernel::getExecutableMemorySize())
	{
		Kernel::setExecutableMemoryOffset(Kernel::getExecutableMemoryOffset() + size);
	}

#endif

	return success;
}

bool Payload::prepare()
{
	bool success;

	xnu::Mach::VmAddress trampoline;

	Task *task = this->getTask();

#if defined(__x86_64__) || (defined(__arm64__) && defined(__USER__))

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

#if defined(__x86_64__) || (defined(__arm64__) && defined(__USER__))

	this->setExecutable();

#endif

	return true;
}
