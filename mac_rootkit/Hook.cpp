#include "Hook.hpp"

#include "Patcher.hpp"
#include "Payload.hpp"

#include "Disassembler.hpp"

#include "Kernel.hpp"
#include "Task.hpp"

#include "Arch.hpp"

using namespace mrk;
using namespace xnu;

Hook::Hook(Patcher *patcher, enum HookType hooktype)
{
	this->patcher = patcher;
	this->hooktype = hooktype;
	this->payload = NULL;
	this->architecture = Arch::initArchitecture();
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
	this->architecture = Arch::initArchitecture();
}

Hook* Hook::hookForFunction(Task *task, Patcher *patcher, mach_vm_address_t address)
{
	Hook *hook;

	std::Array<Hook*> *hooks = patcher->getHooks();

	for(int i = 0; i < hooks->getSize(); i++)
	{
		hook = hooks->get(i);

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

	std::Array<Hook*> *hooks = patcher->getHooks();

	for(int i = 0; i < hooks->getSize(); i++)
	{
		hook = hooks->get(i);

		if(hook->getFrom() == address && hook->getHookType() == kHookTypeBreakpoint)
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

struct HookPatch* Hook::getLatestRegisteredHook()
{
	std::Array<struct HookPatch*> *hooks = this->getHooks();

	if(hooks->getSize() == 0)
		return NULL;

	return hooks->get((int) (hooks->getSize() - 1));
}

mach_vm_address_t Hook::getTrampolineFromChain(mach_vm_address_t address)
{
	std::Array<struct HookPatch*> *hooks = this->getHooks();

	for(int i = 0; i < hooks->getSize(); i++)
	{
		struct HookPatch *patch = hooks->get(i);

		mach_vm_address_t to = patch->to;
		mach_vm_address_t trampoline = patch->trampoline;

		if(to == address)
		{
		#ifdef __arm64__

			__asm__ volatile("PACIZA %[pac]" : [pac] "+rm" (trampoline));

		#endif
			
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

		mach_vm_address_t cb = pair->first;

		if(callback == cb)
		{
			return pair->second;
		}
	}

	return kHookTypeNone;
}

Payload* Hook::prepareTrampoline()
{
	Payload *payload;

	if(this->payload)
	{
		return this->payload;
	}

	this->payload = payload = new Payload(this->getTask(), this, VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE );

	payload->prepare();

	return payload;
}

void Hook::registerHook(struct HookPatch *patch)
{
	this->hooks.add(patch);
}

void Hook::registerCallback(mach_vm_address_t callback, enum HookType hooktype)
{
	HookCallbackPair<mach_vm_address_t, enum HookType> *pair = HookCallbackPair<mach_vm_address_t, enum HookType>::create(callback, hooktype);

	this->callbacks.add(pair);
}

void Hook::hookFunction(mach_vm_address_t to, enum HookType hooktype)
{
	struct HookPatch *hook = new HookPatch;

	Architecture *architecture = this->getArchitecture();

	Disassembler *disassembler = this->getDisassembler();

	Patcher *patcher = this->getPatcher();

	Payload *payload = this->prepareTrampoline();

	struct HookPatch *chain = this->getLatestRegisteredHook();

	mach_vm_address_t trampoline;

	// if we don't have any entries in the chain
	// then we start at the payload's starting point

	// if we have entries in the chain, then start at the correct offset

	if(!chain)
	{
		this->trampoline = trampoline = payload->getAddress();
	} else
	{
		trampoline = payload->getAddress() + payload->getCurrentOffset();
	}

	size_t min;
	
	size_t branch_size;

	mach_vm_address_t from = chain ? chain->to : this->from;

	branch_size = architecture->getBranchSize();

	min = disassembler->instructionSize(from, branch_size);

	if(!min)
	{
		MAC_RK_LOG("Cannot hook! Capstone failed!\n");
		
		return;
	}

	uint8_t *original_opcodes;
	uint8_t *replace_opcodes;

	original_opcodes = new uint8_t[min];

	this->task->read(from, (void*) original_opcodes, min);

	payload->writeBytes(original_opcodes, min);

	union FunctionJmp to_hook_function;

	// build the FunctionPatch branch/jmp instruction from original function to hooked function
	// NOTE: if function is hooked more than once, then original = previous hook

	architecture->makeJmp(&to_hook_function, to, from);

	replace_opcodes = new uint8_t[branch_size];

	memcpy(replace_opcodes, (void*) &to_hook_function, branch_size);

	union FunctionJmp to_original_function;

	// build the FunctionPatch branch/jmp instruction from trampoline to original function

	architecture->makeJmp(&to_original_function, from + min, payload->getAddress() + payload->getCurrentOffset());
	
	payload->writeBytes((uint8_t*) &to_original_function, branch_size);

	this->task->write(from, (void*) &to_hook_function, branch_size);

	hook->from = from;
	hook->to = to;

	hook->trampoline = trampoline;
	hook->patch = to_hook_function;
	hook->payload = payload;
	hook->type = hooktype;

	hook->original = original_opcodes;
	hook->replace = replace_opcodes;
	hook->patch_size = branch_size;

	this->registerHook(hook);
}

void Hook::uninstallHook()
{
}

void Hook::addBreakpoint(mach_vm_address_t breakpoint_hook, enum HookType hooktype)
{
	struct HookPatch *hook = new HookPatch;

	Architecture *architecture = this->getArchitecture();

	Disassembler *disassembler = this->getDisassembler();

	Patcher *patcher = this->getPatcher();

	Payload *payload = this->prepareTrampoline();

	mach_vm_address_t trampoline;

	trampoline = payload->getAddress() + payload->getCurrentOffset();

	size_t min;
	size_t branch_size;

	mach_vm_address_t from = this->from;

	branch_size = architecture->getBranchSize();

	min = disassembler->instructionSize(from, branch_size);

	uint8_t *original_opcodes;
	uint8_t *replace_opcodes;

	original_opcodes = new uint8_t[min];

	this->task->read(from, (void*) original_opcodes, min);

	union FunctionJmp to_trampoline;

	// build the FunctionPatch branch/jmp instruction from original function to hooked function
	// NOTE: if function is hooked more than once, then original = previous hook

	architecture->makeJmp(&to_trampoline, trampoline, from);

	replace_opcodes = new uint8_t[branch_size];

	memcpy(replace_opcodes, (void*) &to_trampoline, branch_size);

	size_t breakpoint_size = architecture->getBreakpointSize();

	if(breakpoint_hook)
	{
		// set a conditional breakpoint
		union FunctionCall call_breakpoint_hook;

		union Breakpoint breakpoint;

		size_t call_size = architecture->getCallSize();

		payload->writeBytes((uint8_t*) push_registers, (size_t) ((uint8_t*) push_registers_end - (uint8_t*) push_registers));
		payload->writeBytes((uint8_t*) set_argument, (size_t) ((uint8_t*) set_argument_end - (uint8_t*) set_argument));

		architecture->makeCall(&call_breakpoint_hook, breakpoint_hook, payload->getAddress() + payload->getCurrentOffset());

		payload->writeBytes((uint8_t*) &call_breakpoint_hook, call_size);

		payload->writeBytes((uint8_t*) check_breakpoint, (size_t) ((uint8_t*) check_breakpoint_end - (uint8_t*) check_breakpoint));

		architecture->makeBreakpoint(&breakpoint);

		payload->writeBytes((uint8_t*) &breakpoint, breakpoint_size);

		payload->writeBytes((uint8_t*) pop_registers, (size_t) ((uint8_t*) pop_registers_end - (uint8_t*) pop_registers));
	} else
	{
		// break regardless
		union Breakpoint breakpoint;

		payload->writeBytes((uint8_t*) push_registers, (size_t) ((uint8_t*) push_registers_end - (uint8_t*) push_registers));
		
		architecture->makeBreakpoint(&breakpoint);

		payload->writeBytes((uint8_t*) &breakpoint, breakpoint_size);

		payload->writeBytes((uint8_t*) pop_registers, (size_t) ((uint8_t*) pop_registers_end - (uint8_t*) pop_registers));
	}

	union FunctionJmp to_original_function;

	// build the FunctionPatch branch/jmp instruction from trampoline to original function

	payload->writeBytes(original_opcodes, min);

	architecture->makeJmp(&to_original_function, from + min, payload->getAddress() + payload->getCurrentOffset());

	payload->writeBytes((uint8_t*) &to_original_function, branch_size);

	this->task->write(from, (void*) replace_opcodes, branch_size);

	hook->from = from;
	hook->to = trampoline;

	hook->trampoline = trampoline;
	hook->patch = to_trampoline;
	hook->payload = payload;
	hook->type = hooktype;

	hook->original = original_opcodes;
	hook->replace = replace_opcodes;
	hook->patch_size = branch_size;

	this->registerHook(hook);
}

void Hook::removeBreakpoint()
{
}
