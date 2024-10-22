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

#include <capstone/capstone.h>

#include "macho.h"

class MachO;

namespace arch {
namespace x86_64 {
namespace patchfinder {
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
                                       UInt8* insn);
xnu::mach::VmAddress FindInstructionBack64(MachO* macho, xnu::mach::VmAddress start, Size length,
                                           UInt8* insn);
xnu::mach::VmAddress FindInstructionNTimes64(MachO* macho, int n, xnu::mach::VmAddress start,
                                             Size length, UInt8* insn, bool forward);

xnu::mach::VmAddress Step64(MachO* macho, xnu::mach::VmAddress start, Size length, char* mnemonic,
                            char* op_string);
xnu::mach::VmAddress StepBack64(MachO* macho, xnu::mach::VmAddress start, Size length,
                                char* mnemonic, char* op_string);

xnu::mach::VmAddress FindFunctionBegin(MachO* macho, xnu::mach::VmAddress start,
                                       xnu::mach::VmAddress where);

xnu::mach::VmAddress FindReference(MachO* macho, xnu::mach::VmAddress to, int n,
                                   enum text which_text);
xnu::mach::VmAddress FindDataReference(MachO* macho, xnu::mach::VmAddress to, enum data which_data,
                                       int n);

uint8_t* FindString(MachO* macho, char* string, xnu::mach::VmAddress base, Size size,
                    Bool full_match);
xnu::mach::VmAddress FindStringReference(MachO* macho, char* string, int n,
                                         enum string which_string, enum text which_text,
                                         Bool full_match);

void PrintInstruction64(MachO* macho, xnu::mach::VmAddress start, UInt32 length, char* mnemonic,
                        char* op_string);
} // namespace patchfinder
} // namespace x86_64
}; // namespace arch
