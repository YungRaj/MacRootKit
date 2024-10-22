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

#include <types.h>

#include <mach/kmod.h>

#include "pair.h"
#include "vector.h"

namespace darwin {
class Hook;

class Patcher {
public:
    explicit Patcher() {}

    ~Patcher() = default;

    virtual void FindAndReplace(void* data, Size data_size, const void* find, Size find_size,
                                const void* replace, Size replace_size);

    virtual void OnKextLoad(void* kext, kmod_info_t* kmod);

    virtual void RouteFunction(darwin::Hook* hook);

    std::vector<Hook*>& GetHooks() {
        return hooks;
    }

    darwin::Hook* HookForFunction(xnu::mach::VmAddress address);

    darwin::Hook* BreakpointForAddress(xnu::mach::VmAddress address);

    bool IsFunctionHooked(xnu::mach::VmAddress address);

    bool IsBreakpointAtInstruction(xnu::mach::VmAddress address);

    void InstallHook(darwin::Hook* hook, xnu::mach::VmAddress hooked);

    void RemoveHook(darwin::Hook* hook);

private:
    std::vector<darwin::Hook*> hooks;
};

} // namespace darwin
