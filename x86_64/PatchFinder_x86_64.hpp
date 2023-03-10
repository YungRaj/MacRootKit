#ifndef __PATCHFINDER_X86_64_HPP_
#define __PATCHFINDER_X86_64_HPP_

#include <capstone/capstone.h>

#include "MachO.hpp"

class MachO;

namespace Arch
{
	namespace x86_64
	{
		namespace PatchFinder
		{
			enum text : int
			{
				__UNKNOWN_TEXT = -1,
				__NO_TEXT = 0,
				__TEXT_XNU_BASE,
				__TEXT_PRELINK_BASE,
				__TEXT_PPL_BASE,
				__TEXT_KLD_BASE,
			};

			enum data : int
			{
				__UNKNOWN_DATA = -1,
				__DATA_CONST = 0, 
				__PPLDATA_CONST,
				__PPLDATA,
				__DATA,
				__BOOTDATA,
				__PRELINK_DATA,
				 __PLK_DATA_CONST,
			};

			enum string : int
			{
				__unknown_ = -1,
				__no_string_ = 0,
				__cstring_,
				__pstring_,
				__oslstring_,
				__data_,
				__const_,
			};
			
			unsigned char* boyermoore_horspool_memmem(const unsigned char* haystack, size_t hlen,
													  const unsigned char* needle,   size_t nlen);

			mach_vm_address_t xref64(MachO *macho, mach_vm_address_t start, mach_vm_address_t end, mach_vm_address_t what);
	
			mach_vm_address_t findInstruction64(MachO *macho, mach_vm_address_t start, size_t length, uint8_t *insn);
			mach_vm_address_t findInstructionBack64(MachO *macho, mach_vm_address_t start, size_t length, uint8_t *insn);
			mach_vm_address_t findInstructionNTimes64(MachO *macho, int n, mach_vm_address_t start, size_t length, uint8_t *insn, bool forward);

			mach_vm_address_t step64(MachO *macho, mach_vm_address_t start, size_t length, char *mnemonic, char *op_string);
			mach_vm_address_t stepBack64(MachO *macho, mach_vm_address_t start, size_t length, char *mnemonic, char *op_string);

			mach_vm_address_t findFunctionBegin(MachO *macho, mach_vm_address_t start, mach_vm_address_t where);

			mach_vm_address_t findReference(MachO *macho, mach_vm_address_t to, int n, enum text which_text);
			mach_vm_address_t findDataReference(MachO *macho, mach_vm_address_t to, enum data which_data, int n);

			uint8_t* findString(MachO *macho, char *string, mach_vm_address_t base, mach_vm_address_t size, bool full_match);
			mach_vm_address_t findStringReference(MachO *macho, char *string, int n, enum string which_string, enum text which_text, bool full_match);

			void printInstruction64(MachO *macho, mach_vm_address_t start, uint32_t length, char *mnemonic, char *op_string);
		}
	}
};

#endif