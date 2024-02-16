#ifndef __TYPES_HPP__
#define __TYPES_HPP__

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <mach/mach_types.h>
#include <mach/vm_types.h>

#include <mach/kmod.h>

namespace mach
{
	typedef mach_vm_address_t 		VmAddress;
	typedef mach_port_t       		Port;
};

namespace xnu
{
	typedef size_t 					Size;
	
	typedef vm_prot_t  				VmProtection;

	typedef vm_map_ 				Vmap;
	typedef pmap_t 					Pmap;

	typedef task_t 					Task;
	typedef proc_t 					Proc;
};

namespace macho
{
	typedef struct mach_header_64   Header64;

	typedef struct nlist_64 		Nlist64;

	typedef char* 					SymName;
};

#endif