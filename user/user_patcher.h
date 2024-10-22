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

#include "patcher.h"

namespace darwin {
class Hook;
class Payload;

class UserPatcher : Patcher {
public:
    explicit UserPatcher() = default;

    ~UserPatcher() = default;

    virtual void FindAndReplace(void* data, Size data_size, const void* find, Size find_size,
                                const void* replace, Size replace_size);

    virtual void RouteFunction(Hook* hook);

    virtual void OnExec(char* name, int pid, xnu::mach::Port port, xnu::mach::VmAddress task,
                        xnu::mach::VmAddress proc);

    virtual void OnKextLoad(void* kext, kmod_info_t* kmod);

    xnu::mach::VmAddress InjectPayload(xnu::mach::VmAddress address, Payload* payload);

    xnu::mach::VmAddress InjectSegment(xnu::mach::VmAddress address, Payload* payload);

    Size MapAddresses(const char* mapBuf);

private:
};
} // namespace darwin
