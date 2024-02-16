#ifndef __TYPES_HPP__
#define __TYPES_HPP__

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <mach/mach_types.h>
#include <mach/vm_types.h>

#include <mach/kmod.h>

namespace xnu
{
	using Size = size_t;

	using Task 	= task_t;
	using Proc 	= proc_t;

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

#endif