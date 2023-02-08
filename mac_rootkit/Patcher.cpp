#include "Patcher.hpp"

#include "Hook.hpp"

using namespace mrk;

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
	hooks.add(hook);
}

bool Patcher::isFunctionHooked(mach_vm_address_t address)
{
	for(int i = 0; i < this->getHooks()->getSize(); i++)
	{
		Hook *hook = this->getHooks()->get(i);

		if(hook->getHookType() == kHookTypeInstrumentFunction ||
		   hook->getHookType() == kHookTypeReplaceFunction)
		{
			if(hook->getFrom() == address)
			{
				return true;
			}
		}
	}

	return false;
}

bool Patcher::isBreakpointAtInstruction(mach_vm_address_t address)
{
	for(int i = 0; i < this->getHooks()->getSize(); i++)
	{
		Hook *hook = this->getHooks()->get(i);

		if(hook->getHookType() == kHookTypeBreakpoint)
		{
			if(hook->getFrom() == address)
			{
				return true;
			}
		}
	}

	return false;
}

Hook* Patcher::hookForFunction(mach_vm_address_t address)
{
	Hook *hook = NULL;

	if(!this->isFunctionHooked(address))
		return NULL;

	for(int i = 0; i < this->getHooks()->getSize(); i++)
	{
		Hook *h = this->getHooks()->get(i);

		if(h->getHookType() == kHookTypeInstrumentFunction ||
		   h->getHookType() == kHookTypeReplaceFunction)
		{
			if(hook->getFrom() == address)
			{
				hook = h;
			}
		}
	}

	return hook;
}

Hook* Patcher::breakpointForAddress(mach_vm_address_t address)
{
	Hook *hook = NULL;

	if(!this->isBreakpointAtInstruction(address))
		return NULL;

	for(int i = 0; i < this->getHooks()->getSize(); i++)
	{
		Hook *h = this->getHooks()->get(i);

		if(h->getHookType() == kHookTypeBreakpoint)
		{
			if(hook->getFrom() == address)
			{
				hook = h;
			}
		}
	}

	return hook;
}

void Patcher::installHook(Hook *hook, mach_vm_address_t hooked)
{
	hook->hookFunction(hooked);

	if(!this->hooks.find(hook))
	{
		this->hooks.add(hook);
	}
}

void Patcher::removeHook(Hook *hook)
{
	hook->uninstallHook();

	this->hooks.remove(hook);

	delete hook;
}