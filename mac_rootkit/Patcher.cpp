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
							Size data_size,
							const void *find, Size find_size,
							const void *replace, Size replace_size)
{
}

void Patcher::onKextLoad(void *kext, kmod_info_t *kmod)
{
}

void Patcher::routeFunction(Hook *hook)
{
	hooks.push_back(hook);
}

bool Patcher::isFunctionHooked(xnu::Mach::VmAddress address)
{
	for(int i = 0; i < this->getHooks().size(); i++)
	{
		Hook *hook = this->getHooks().at(i);

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

bool Patcher::isBreakpointAtInstruction(xnu::Mach::VmAddress address)
{
	for(int i = 0; i < this->getHooks().size(); i++)
	{
		Hook *hook = this->getHooks().at(i);

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

Hook* Patcher::hookForFunction(xnu::Mach::VmAddress address)
{
	Hook *hook = NULL;

	if(!this->isFunctionHooked(address))
		return NULL;

	for(int i = 0; i < this->getHooks().size(); i++)
	{
		Hook *h = this->getHooks().at(i);

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

Hook* Patcher::breakpointForAddress(xnu::Mach::VmAddress address)
{
	Hook *hook = NULL;

	if(!this->isBreakpointAtInstruction(address))
		return NULL;

	for(int i = 0; i < this->getHooks().size(); i++)
	{
		Hook *h = this->getHooks().at(i);

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

void Patcher::installHook(Hook *hook, xnu::Mach::VmAddress hooked)
{
	hook->hookFunction(hooked);

	if(std::find(hooks.begin(), hooks.end(), hook) != hooks.end())
	{
		this->hooks.push_back(hook);
	}
}

void Patcher::removeHook(Hook *hook)
{
	hook->uninstallHook();

	this->hooks.erase(std::remove(hooks.begin(), hooks.end(), hook), hooks.end());

	delete hook;
}