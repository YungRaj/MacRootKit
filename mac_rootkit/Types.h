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

#include <mach/mach_types.h>
#include <mach/vm_types.h>

#include <mach/kmod.h>

using Bool = bool;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

namespace xnu
{
	using Offset = off_t;
	using Size = size_t;

	using Task 	= task_t;
	using Proc 	= proc_t;

	using KmodInfo = kmod_info_t;

	using KmodStartFunc = kmod_start_func_t;
	using KmodStopFunc = kmod_stop_func_t;

	namespace Mach
	{
		using VmMap = vm_map_t;
		using Pmap  = pmap_t;

		using VmAddress    = mach_vm_address_t;
		using VmProtection = vm_prot_t;

		using Port 		= mach_port_t;
	};

	namespace MachO
	{
		using Header64 = mach_header_64;

		using Nlist64 = struct nlist_64;

		using SymbolName = char*;

		using SegmentName = char*;
		using SectionName = char*;
	};
};