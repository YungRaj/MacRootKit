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

#include <Types.h>

#include <mach/kmod.h>

#include "Pair.hpp"
#include "vector.hpp"

namespace mrk {
class Hook;

class Patcher {
public:
    Patcher();

    ~Patcher();

    virtual void findAndReplace(void* data, Size data_size, const void* find, Size find_size,
                                const void* replace, Size replace_size);

    virtual void onKextLoad(void* kext, kmod_info_t* kmod);

    virtual void routeFunction(mrk::Hook* hook);

    std::vector<Hook*>& getHooks() {
        return hooks;
    }

    mrk::Hook* hookForFunction(xnu::Mach::VmAddress address);

    mrk::Hook* breakpointForAddress(xnu::Mach::VmAddress address);

    bool isFunctionHooked(xnu::Mach::VmAddress address);

    bool isBreakpointAtInstruction(xnu::Mach::VmAddress address);

    void installHook(mrk::Hook* hook, xnu::Mach::VmAddress hooked);

    void removeHook(mrk::Hook* hook);

private:
    std::vector<mrk::Hook*> hooks;
};

} // namespace mrk
