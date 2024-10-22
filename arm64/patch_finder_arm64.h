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

#include "machO.h"

class MachO;

namespace arch {
namespace arm64 {
namespace patchfinder {
#define NO_REG -1

enum text : int {
    __UNKNOWN_TEXT = -1,
    __NO_TEXT = 0,
    __TEXT_XNU_BASE,
    __TEXT_PRELINK_BASE,
    __TEXT_PPL_BASE,
    __TEXT_KLD_BASE,
};

enum data : int {
    __UNKNOWN_DATA = -1,
    __DATA_CONST = 0,
    __PPLDATA_CONST,
    __PPLDATA,
    __DATA,
    __BOOTDATA,
    __PRELINK_DATA,
    __PLK_DATA_CONST,
};

enum string : int {
    __unknown_ = -1,
    __no_string_ = 0,
    __cstring_,
    __pstring_,
    __oslstring_,
    __data_,
    __const_,
};

unsigned char* boyermoore_horspool_memmem(const unsigned char* haystack, Size hlen,
                                          const unsigned char* needle, Size nlen);

xnu::mach::VmAddress Xref64(MachO* macho, xnu::mach::VmAddress start, xnu::mach::VmAddress end,
                            xnu::mach::VmAddress what);

xnu::mach::VmAddress FindInstruction64(MachO* macho, xnu::mach::VmAddress start, Size length,
                                       UInt32 ins);
xnu::mach::VmAddress FindInstructionBack64(MachO* macho, xnu::mach::VmAddress start, Size length,
                                           UInt32 ins);
xnu::mach::VmAddress FindInstructionNTimes64(MachO* macho, int n, xnu::mach::VmAddress start,
                                             Size length, UInt32 ins, bool forward);

xnu::mach::VmAddress Step64(MachO* macho, xnu::mach::VmAddress start, Size length,
                            bool (*is_ins)(UInt32*), int Rt, int Rn);
xnu::mach::VmAddress StepBack64(MachO* macho, xnu::mach::VmAddress start, Size length,
                                bool (*is_ins)(UInt32*), int Rt, int Rn);

xnu::mach::VmAddress FindFunctionBegin(MachO* macho, xnu::mach::VmAddress start,
                                       xnu::mach::VmAddress where);

xnu::mach::VmAddress FindReference(MachO* macho, xnu::mach::VmAddress to, int n,
                                   enum text which_text);
xnu::mach::VmAddress FindDataReference(MachO* macho, xnu::mach::VmAddress to, enum data which_data,
                                       int n);

uint8_t* FindString(MachO* macho, char* string, xnu::mach::VmAddress base,
                    xnu::mach::VmAddress size, bool full_match);
xnu::mach::VmAddress FindStringReference(MachO* macho, char* string, int n,
                                         enum string which_string, enum text which_text,
                                         bool full_match);

void PrintInstruction64(MachO* macho, xnu::mach::VmAddress start, UInt32 length,
                        bool (*is_ins)(UInt32*), int Rt, int Rn);
} // namespace patchfinder
} // namespace arm64
}; // namespace arch
