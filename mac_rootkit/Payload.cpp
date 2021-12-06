#include "Payload.hpp"

#include "Hook.hpp"

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

	success = this->readBytes(this->current_offset, size);

	return success;
}

bool Payload::readBytes(off_t offset, uint8_t *bytes, size_t size)
{
	bool success;

	success = this->getTask()->read(this->address + offset, (void*) bytes, size);

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

	success = this->getTask()->write(this->address + offset, (void*) bytes, size);

	return success;
}

bool Payload::prepare()
{
	bool success;

	mach_vm_address_t trampoline;

	Task *task = this->getTask();

	trampoline = task->vmAllocate(Payload::expectedSize, VM_FLAGS_ANYWHERE, VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE);

	if(!address)
		return false;

	this->address = trampoline;
}
	