#include "Hook.hpp"

#include "Patcher.hpp"
#include "Payload.hpp"

#include "Disassembler.hpp"

#include "Kernel.hpp"
#include "Task.hpp"

#include "Arch.hpp"

Hook::Hook(Patcher *patcher, enum HookType hooktype)
{
	this->patcher = patcher;
	this->hooktype = hooktype;
	this->payload = NULL;
}

Hook::Hook(Patcher *patcher, enum HookType hooktype, Task *task, mach_vm_address_t from)
{
	this->patcher = patcher;
	this->hooktype = hooktype;
	this->payload = NULL;

	this->task = task;
	this->from = from;

	if(this->hooktype == kHookTypeInstrumentFunction)
		this->initWithHookParams(task, from);
	if(this->hooktype == kHookTypeBreakpoint)
		this->initWithBreakpointParams(task, from);

	this->disassembler = this->task->getDisassembler();
}

Hook* Hook::hookForFunction(Task *task, Patcher *patcher, mach_vm_address_t address)
{
	Hook *hook;

	Array<Hook*> *hooks = patcher->getHooks();

	for(int i = 0; i < hooks->getSize(); i++)
	{
		hook = hook->get(i);

		if(hook->getFrom() == address && hook->getHookType() == kHookTypeInstrumentFunction)
		{
			return hook;
		}
	}

	hook = new Hook(patcher, kHookTypeInstrumentFunction);

	hook->initWithHookParams(task, address);

	return hook;
}

Hook* Hook::breakpointForAddress(Task *task, Patcher *patcher, mach_vm_address_t address)
{
	Hook *hook;

	Array<Hook*> *hooks = patcher->getHooks();

	for(int i = 0; i < hooks->getSize(); i++)
	{
		hook = hooks->get(i);

		if(hook->getFrom() == address && hook->getType() == kHookTypeBreakpoint)
		{
			return hook;
		}
	}

	hook = new Hook(patcher, kHookTypeBreakpoint);

	hook->initWithBreakpointParams(task, address);

	return hook;
}

void Hook::initWithHookParams(Task *task, mach_vm_address_t from)
{
	this->setTask(task);
	this->setFrom(from);
	this->setDisassembler(task->getDisassembler());
}

void Hook::initWithBreakpointParams(Task *task, mach_vm_address_t breakpoint)
{
	this->setHookType(kHookTypeBreakpoint);
	this->setTask(task);
	this->setFrom(breakpoint);
	this->setDisassembler(task->getDisassembler());
}

struct HookPatch* getLatestRegisteredHook()
{
	Array<struct HookPatch*> *hooks = this->getHooks();

	if(hooks->getSize() == 0)
		return NULL;

	return hooks->get(hooks->getSize() - 1):
}

mach_vm_address_t Hook::getTrampolineFromChain(mach_vm_address_t address)
{
	Array<struct HookPatch*> *hooks = this->getHooks();

	for(int i = 0; i < hooks; i++)
	{
		struct HookPatch *patch = hooks->get(i):

		mach_vm_address_t to = patch->to;
		mach_vm_address_t trampoline = patch->trampoline;

		if(to == address)
		{
			return trampoline;
		}
	}

	return 0;
}

enum HookType Hook::getHookTypeForCallback(mach_vm_address_t callback)
{
	for(int i = 0; i < this->getCallbacks()->getSize(); i++)
	{
		auto pair = this->getCallbacks()->get(i);

		mach_vm_addresss_t cb = pair->first;

		if(callback == cb)
		{
			return pair->second;
		}
	}

	return kHookTypeNone;
}

void Hook::makePatch(union FunctionPatch *patch, mach_vm_address_t to, mach_vm_address_t from)
{
	switch(Arch::getCurrentArchitecture())
	{
		case ARCH_x86_64:
			patch->patch_x86_64 = Arch::x86_64::makeJmp(to, from);

			break;
		case ARCH_arm64:
			patch->patch_arm64 = Arch::arm64::makeBranch(to, from);

			break;

		default:
			break;
	}
}

void Hook::makeCall(union FunctionCall *call, mach_vm_address_t to, mach_vm_address_t from)
{
	switch(Arch::getCurrentArchitecture())
	{
		case ARCH_x86_64:
			call->call_x86_64 = Arch::x86_64::makeCall(to, from);

			break;
		case ARCH_arm64:
			call->call_arm64 = Arch::arm64::makeCall(to, from);

			break;

		default:
			break;
	}
}

void Hook::makeBreakpoint(union Breakpoint *breakpoint)
{
	switch(Arch::getCurrentArchitecture())
	{
		case ARCH_x86_64:
			breakpoint->breakpoint_x86_64 = Arch::x86_64::makeBreakpoint();

			break;
		case ARCH_arm64:
			breakpoint->breakpoint_arm64 = Arch::arm64::makeBreakpoint();

			break;

		default:
			break;
	}
}

Payload* Hook::prepareTrampoline()
{
}

void Hook::registerHook(struct HookPatch *patch)
{
}

void Hook::registerCallback(mach_vm_address_t callback, enum HookType hooktype = kHookTypeCallback)
{
}

void Hook::hookFunction(mach_vm_address_t to, enum HookType hooktype = kHookTypeInstrumentFunction)
{
}

void Hook::uninstallHook()
{
}

void Hook::addBreakpoint(mach_vm_address_t breakpoint_hook, enum HookType hooktype = kHookTypeBreakpoint)
{
}

void Hook::removeBreakpoint()
{
}
