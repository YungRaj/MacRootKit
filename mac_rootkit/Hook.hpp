/*
 * Copyright (c) YungRaj
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "Kernel.hpp"
#include "Patcher.hpp"

#include "Arch.hpp"

#include "vector.hpp"

#include <Types.h>

#include <arm64/Isa_arm64.hpp>
#include <x86_64/Isa_x86_64.hpp>

namespace mrk {
class MacRootKit;

class Patcher;

class Payload;
}; // namespace mrk

namespace xnu {
class Kernel;
class Kext;

class Task;
} // namespace xnu

using namespace Arch;

enum HookType {
    kHookTypeNone,
    kHookTypeBreakpoint,
    kHookTypeCallback,
    kHookTypeInstrumentFunction,
    kHookTypeReplaceFunction,
};

struct HookPatch {
    xnu::Mach::VmAddress to;
    xnu::Mach::VmAddress from;

    xnu::Mach::VmAddress trampoline;

    enum HookType type;

    union Branch patch;

    mrk::Payload* payload;

    UInt8* original;
    UInt8* replace;

    Size patch_size;
};

template <typename T, typename Y = enum HookType>
using HookCallbackPair = Pair<T, Y>;

template <typename T, typename Y = enum HookType>
using HookCallbackArray = std::vector<HookCallbackPair<T, Y>*>;

template <typename T = struct HookPatch*>
using HookArray = std::vector<T>;

namespace mrk {
class Hook {
public:
    explicit Hook(mrk::Patcher* patcher, enum HookType hooktype);
    explicit Hook(mrk::Patcher* patcher, enum HookType hooktype, xnu::Task* task,
                  xnu::Mach::VmAddress from);

    static Hook* hookForFunction(xnu::Task* task, mrk::Patcher* patcher,
                                 xnu::Mach::VmAddress address);
    static Hook* hookForFunction(void* target, xnu::Task* task, mrk::Patcher* patcher,
                                 xnu::Mach::VmAddress address);

    static Hook* breakpointForAddress(xnu::Task* task, mrk::Patcher* patcher,
                                      xnu::Mach::VmAddress address);
    static Hook* breakpointForAddress(void* target, xnu::Task* task, mrk::Patcher* patcher,
                                      xnu::Mach::VmAddress address);

    void* getTarget() {
        return target;
    }

    mrk::Patcher* getPatcher() {
        return patcher;
    }

    xnu::Task* getTask() {
        return task;
    }

    Architecture* getArchitecture() {
        return architecture;
    }

    Disassembler* getDisassembler() {
        return disassembler;
    }

    xnu::Mach::VmAddress getFrom() {
        return from;
    }

    struct HookPatch* getLatestRegisteredHook();

    xnu::Mach::VmAddress getTrampoline() {
        return trampoline;
    }

    xnu::Mach::VmAddress getTrampolineFromChain(xnu::Mach::VmAddress address);

    HookArray<struct HookPatch*>& getHooks() {
        return hooks;
    }

    HookCallbackArray<xnu::Mach::VmAddress>& getCallbacks() {
        return callbacks;
    }

    enum HookType getHookType() {
        return hooktype;
    }

    enum HookType getHookTypeForCallback(xnu::Mach::VmAddress callback);

    void setTarget(void* target) {
        this->target = target;
    }

    void setPatcher(Patcher* patcher) {
        this->patcher = patcher;
    }

    void setDisassembler(Disassembler* disassembler) {
        this->disassembler = disassembler;
    }

    void setTask(Task* task) {
        this->task = task;
    }

    void setFrom(xnu::Mach::VmAddress from) {
        this->from = from;
    }

    void setTrampoline(xnu::Mach::VmAddress trampoline) {
        this->trampoline = trampoline;
    }

    void setHookType(enum HookType hooktype) {
        this->hooktype = hooktype;
    }

    void prepareHook(xnu::Task* task, xnu::Mach::VmAddress from);
    void prepareBreakpoint(xnu::Task* task, xnu::Mach::VmAddress breakpoint);

    mrk::Payload* prepareTrampoline();

    void registerHook(struct HookPatch* patch);

    void registerCallback(xnu::Mach::VmAddress callback,
                          enum HookType hooktype = kHookTypeCallback);

    void hookFunction(xnu::Mach::VmAddress to,
                      enum HookType hooktype = kHookTypeInstrumentFunction);

    void uninstallHook();

    void addBreakpoint(xnu::Mach::VmAddress breakpoint_hook,
                       enum HookType hooktype = kHookTypeBreakpoint);

    void removeBreakpoint();

private:
    void* target;

    mrk::Patcher* patcher;

    xnu::Task* task;

    Arch::Architecture* architecture;

    Disassembler* disassembler;

    mrk::Payload* payload;

    bool kernelHook = false;

    xnu::Mach::VmAddress from;

    xnu::Mach::VmAddress trampoline;

    enum HookType hooktype;

    HookCallbackArray<xnu::Mach::VmAddress> callbacks;

    HookArray<struct HookPatch*> hooks;
};

} // namespace mrk
