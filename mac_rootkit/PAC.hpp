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

#include <mach/mach_types.h>

#include <ptrauth.h>

#include "Arch.hpp"

#ifdef defined(__arm64__) || defined(__arm64e__)

static_assert(Arch::_arm64<Arch::getCurrentArchitecture()>);

namespace Arch {
namespace arm64 {
namespace PAC {
UInt64 signPointerWithAKey(UInt64 pointer);

UInt64 signPointerWithBKey(UInt64 pointer);

void stripPointerAuthenticationCode(UInt64 pointer);
} // namespace PAC
} // namespace arm64
}; // namespace Arch

#define PACSignPointerWithAKey(ptr) PAC::signPointerWithAKey(ptr)
#define PACSignPointerWithBKey(ptr) PAC::signPointerWithBKey(ptr);

#else

#define PACSignPointerWithAKey(ptr) ptr
#define PACSignPointerWithBKey(ptr) ptr

#endif