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

#include "arch.h"

class MachO;

class Segment;
class Section;

class Symbol;

namespace darwin {
class DarwinKit;
};

namespace xnu {
class Kext;
class Kernel;
} // namespace xnu

darwin::DarwinKit* darwinkit_get_darwinkit();

namespace debug {
namespace symbolicate {
void LookForAddressInsideKernel(xnu::mach::VmAddress address, xnu::Kernel* kernel, Symbol** sym,
                                Offset* delta);
void LookForAddressInsideKexts(xnu::mach::VmAddress address, std::vector<xnu::Kext*>& kexts,
                               Symbol** sym, Offset* delta);

Symbol* GetSymbolFromAddress(xnu::mach::VmAddress address, Offset* delta);
}; // namespace symbolicate

void PrintBacktrace();
}; // namespace debug
