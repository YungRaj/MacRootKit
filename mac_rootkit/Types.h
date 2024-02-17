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

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <sys/types.h>

#include <mach/mach_types.h>
#include <mach/vm_types.h>

#include <mach/kmod.h>

#include <mach-o.h>

using Bool = bool;

using Int8 = int8_t;
using Int16 = int16_t;
using Int32 = int32_t;
using Int64 = int64_t;

using UInt8 = uint8_t;
using UInt16 = uint16_t;
using UInt32 = uint32_t;
using UInt64 = uint64_t;

using Offset = off_t;

#ifdef __KERNEL__
using Size = size_t;
#elif __USER__
#include <IOKit/IOKitLib.h>
#endif

namespace xnu
{
	using KmodInfo = kmod_info_t;

	using KmodStartFunc = kmod_start_func_t;
	using KmodStopFunc = kmod_stop_func_t;

	namespace Mach
	{
		using VmMap = vm_map_t;

		using VmAddress    = mach_vm_address_t;
		using VmProtection = vm_prot_t;

		using Port 		= mach_port_t;
	};

	namespace Macho
	{
		using Header64 = struct mach_header_64;

		using Nlist64 = struct nlist_64;

		using SymbolName = char*;

		using SegmentName = char*;
		using SectionName = char*;
	};
};