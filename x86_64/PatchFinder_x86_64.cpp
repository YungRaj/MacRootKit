#include "PatchFinder_x86_64.hpp"

namespace Arch
{
	namespace x86_64
	{
		namespace PatchFinder
		{
			mach_vm_address_t xref64(MachO *macho, mach_vm_address_t start, mach_vm_address_t end, mach_vm_address_t what)
			{
				return 0;
			}
	
			mach_vm_address_t findInstruction64(MachO *macho, mach_vm_address_t start, size_t length, uint32_t ins)
			{
				return 0;
			}

			mach_vm_address_t findInstructionBack64(MachO *macho, mach_vm_address_t start, size_t length, uint32_t ins)
			{
				return 0;
			}

			mach_vm_address_t findInstructionNTimes64(MachO *macho, int n, mach_vm_address_t start, size_t length, uint32_t ins, bool forward)
			{
				return 0;
			}

			mach_vm_address_t step64(MachO *macho, mach_vm_address_t start, size_t length, bool (*is_ins)(uint32_t*), int Rt, int Rn)
			{
				return 0;
			}

			mach_vm_address_t stepBack64(MachO *macho, mach_vm_address_t start, size_t length, bool (*is_ins)(uint32_t*), int Rt, int Rn)
			{
				return 0;
			}

			mach_vm_address_t findFunctionBegin(MachO *macho, mach_vm_address_t start, mach_vm_address_t where)
			{
				return 0;
			}

			mach_vm_address_t findReference(MachO *macho, mach_vm_address_t to, int n, enum text which_text)
			{
				return 0;
			}

			mach_vm_address_t findDataReference(MachO *macho, mach_vm_address_t to, enum data which_data, int n)
			{
				return 0;
			}

			uint8_t* findString(MachO *macho, char *string, mach_vm_address_t base, mach_vm_address_t size, bool full_match)
			{
				return NULL;
			}
			
			mach_vm_address_t findStringReference(MachO *macho, char *string, int n, enum string which_string, enum text which_text, bool full_match)
			{
				return 0;
			}

			void printInstruction64(MachO *macho, mach_vm_address_t start, uint32_t length, bool (*is_ins)(uint32_t*), int Rt, int Rn)
			{
				return;
			}

		}
	}
}