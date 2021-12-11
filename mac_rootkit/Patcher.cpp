#include "Patcher.hpp"

Patcher::Patcher()
{
}

Patcher::~Patcher()
{
}

void Patcher::findAndReplace(void *data,
							size_t data_size,
							const void *find, size_t find_size,
							const void *replace, size_t replace_size)
{
}

void Patcher::onKextLoad(void *kext, kmod_info_t *kmod)
{
}

void Patcher::routeFunction(Hook *hook)
{
}

Hook* Patcher::hookForFunction(mach_vm_address_t address)
{
	return NULL;
}

Hook* Patcher::breakpointForAddress(mach_vm_address_t address)
{
	return NULL;
}

bool Patcher::isFunctionHooked(mach_vm_address_t address)
{
	return false;
}

bool Patcher::isBreakpointAtInstruction(mach_vm_address_t address)
{
	return false;
}

void Patcher::installHook(Hook *hook, mach_vm_address_t hooked)
{
}

void Patcher::removeHook(Hook *hook)
{
}