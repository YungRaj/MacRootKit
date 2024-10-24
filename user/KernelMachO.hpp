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

#include <Types.hpp>

#include "MachO.hpp"

namespace xnu {
class Kernel;

class KernelMachO : public MachO {
public:
    KernelMachO() {}

    KernelMachO(UIntPtr address);
    KernelMachO(UIntPtr address, Offset slide);

    KernelMachO(const char* path, Offset slide);
    KernelMachO(const char* path);

    ~KernelMachO();

    virtual void parseLinkedit();

    virtual bool parseLoadCommands();

    virtual void parseMachO();

private:
    xnu::Kernel* kernel;

protected:
    UInt8* linkedit;

    xnu::Mach::VmAddress linkedit_off;

    Size linkedit_size;
};

class KernelCacheMachO : public KernelMachO {
public:
    KernelCacheMachO(xnu::Mach::VmAddress kc, UIntPtr address);
    KernelCacheMachO(xnu::Mach::VmAddress kc, UIntPtr address, Offset slide);

    virtual bool parseLoadCommands();

private:
    xnu::Mach::VmAddress kernel_cache;
};

} // namespace xnu
