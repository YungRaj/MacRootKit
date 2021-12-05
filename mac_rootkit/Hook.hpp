#ifndef __HOOK_HPP_
#define __HOOK_HPP_

#include "Kernel.hpp"
#include "Patcher.hpp"

#include "Arch.hpp"

#include "Array.hpp"

class MacRootKit;

class Patcher;

class Payload;

class Kernel;
class Kext;

class Task;

using namespace Arch;

using RegisterState_x86_64 = struct Arch::x86_64::x86_64_register_state;
using RegisterState_arm64 = struct Arch::arm64::arm64_register_state;

using FunctionPatch_x86_64 = union Arch::x86_64::FunctionPatch;
using FunctionPatch_arm64 = union Arch::arm64::FunctionPatch;

using FunctionCall_x86_64 = union Arch::x86_64::FunctionCall;
using FunctionCall_arm64 = union Arch::arm64::FunctionCall;

using Breakpoint_x86_64 = union Arch::x86_64::Breakpoint;
using Breakpoint_arm64 = union Arch::arm64::Breakpoint;

enum HookType
{
	kHookTypeNone,
	kHookTypeBreakpoint,
	kHookTypeCallback,
	kHookTypeInstrumentFunction,
	kHookTypeReplaceFunction,
};

union RegisterState
{
	RegisterState_x86_64 state_x86_64;
	RegisterState_arm64 state_arm64;
};

union FunctionPatch
{
	FunctionPatch_x86_64 patch_x86_64;
	FunctionPatch_arm64 patch_arm64;
};

union FunctionCall
{
	FunctionCall_x86_64 call_x86_64;
	FunctionCall_arm64 call_arm64;
};

union Breakpoint
{
	Breakpoint_x86_64 breakpoint_x86_64;
	Breakpoint_arm64 breakpoint_arm64;
}

struct HookPatch
{
	mach_vm_address_t to;
	mach_vm_address_t from;

	mach_vm_address_t trampoline;

	enum HookType hooktype;

	union FunctionPatch patch;

	Payload *payload;

	uint8_t *find;
	uint8_t *replace;

	size_t size;
};

class Patcher;

template<typename T, typename Y = enum Hooktype>
using HookCallbackPair = Pair<T, Y>;

template<typename T, typename Y = enum HookType>
using HookCallbackArray = Array<HookCallbackPair<T, Y>*>;

template<typename T = struct HookPatch*>
using HookArray = Array<T>;

class Hook
{
	public:
		Hook(Patcher *patcher, enum HookType hooktype);
		Hook(Patcher *patcher, enum HookType hooktype, Task *task, mach_vm_address_t from);

		void initWithHookParams(Task *task, mach_vm_address_t from);
		void initWithBreakpointParams(Task *task, mach_vm_address_t breakpoint);

		static Hook* hookForFunction(Task *task, Patcher *patcher, mach_vm_address_t address);
		static Hook* breakpointForAddress(Task *task, Patcher *patcher, mach_vm_address_t address);

		Patcher* getPatcher() { return patcher; }

		Task* getTask() { return task; }

		mach_vm_address_t getFrom() { return from; }

		mach_vm_address_t getTrampoline() { return trampoline; }

		mach_vm_address_t getTrampolineFromChain(mach_vm_address_t address);

		HookArray<struct HookPatch*>* getHooks() { return &hooks; }

		HookCallbackArray<mach_vm_address_t>* getCallbacks() { return &callbacks; }

		enum HookType getHookType() { return hooktype; }

		enum HookType getHookTypeForCallback(mach_vm_address_t callback);

		void setPatcher(Patcher *patcher) { this->patcher = patcher; }

		void setTask(Task *task) { this->task = task; }

		void setFrom(mach_vm_address_t from) { this->from = from; }

		void setTrampoline(mach_vm_address_t trampoline) { this->trampoline = trampoline; }

		void setHookType(enum HookType hooktype) { this->hooktype = hooktype; }

		Payload* prepareTrampoline();

		void registerHook(struct HookPatch *patch);

		void registerCallback(mach_vm_address_t callback, enum HookType hooktype = kHookTypeCallback);

		void hookFunction(mach_vm_address_t to, enum HookType hooktype = kHookTypeInstrumentFunction);

		void uninstallHook();

		void addBreakpoint(mach_vm_address_t breakpoint_hook, enum HookType hooktype = kHookTypeBreakpoint);

		void removeBreakpoint();

	private:
		Patcher *patcher;

		Task *task;

		Payload *payload;

		bool kernelHook = false;

		mach_vm_address_t from;

		mach_vm_address_t trampoline;

		enum HookType hooktype;

		HookCallbackArray<mach_vm_address_t> callbacks;

		HookArray<struct HookPatch*> hooks;

	protected:

		void makePatch(union FunctionPatch *patch, mach_vm_address_t to, mach_vm_address_t from);

		void makeCall(union FunctionPatch *call, mach_vm_address_t to, mach_vm_address_t from);

		void makeBreakpoint(union Breakpoint *breakpoint);

};

#endif