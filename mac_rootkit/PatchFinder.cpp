#include "PatchFinder.hpp"

namespace PatchFinder
{
	mach_vm_address_t xref64(MachO *macho, mach_vm_address_t start, mach_vm_address_t end, mach_vm_address_t what)
	{
		enum Architectures architecture = Arch::getCurrentArchitecture();

		switch(architecture)
		{
			case ARCH_x86_64:
				return Arch::x86_64::PatchFinder::xref64(macho, start, end, what);
			case ARCH_arm64:
				return Arch::arm64::PatchFinder::xref64(macho, start, end, what);
			default:
				break;
		}

		return 0;
	}
	
	mach_vm_address_t findInstruction64(MachO *macho, mach_vm_address_t start, size_t length, uint32_t ins)
	{
		enum Architectures architecture = Arch::getCurrentArchitecture();

		switch(architecture)
		{
			case ARCH_x86_64:
				return Arch::x86_64::PatchFinder::findInstruction64(macho, start, length, ins);
			case ARCH_arm64:
				return Arch::arm64::PatchFinder::findInstruction64(macho, start, length, ins);
			default:
				break;
		}

		return 0;
	}

	mach_vm_address_t findInstructionBack64(MachO *macho, mach_vm_address_t start, size_t length, uint32_t ins)
	{
		enum Architectures architecture = Arch::getCurrentArchitecture();

		switch(architecture)
		{
			case ARCH_x86_64:
				return Arch::x86_64::PatchFinder::findInstructionBack64(macho, start, length, ins);
			case ARCH_arm64:
				return Arch::arm64::PatchFinder::findInstructionBack64(macho, start, length, ins);
			default:
				break;
		}

		return 0;
	}

	mach_vm_address_t findInstructionNTimes64(MachO *macho, int n, mach_vm_address_t start, size_t length, uint32_t ins, bool forward)
	{
		enum Architectures architecture = Arch::getCurrentArchitecture();

		switch(architecture)
		{
			case ARCH_x86_64:
				return Arch::x86_64::PatchFinder::findInstructionNTimes64(macho, n, start, length, ins, forward);
			case ARCH_arm64:
				return Arch::arm64::PatchFinder::findInstructionNTimes64(macho, n, start, length, ins, forward);
				break;
		}

		return 0;
	}

	mach_vm_address_t step64(MachO *macho, mach_vm_address_t start, size_t length, bool (*is_ins)(uint32_t*), int Rt, int Rn)
	{
		enum Architectures architecture = Arch::getCurrentArchitecture();

		switch(architecture)
		{
			case ARCH_x86_64:
				return Arch::x86_64::PatchFinder::step64(macho, start, length, is_ins, Rt, Rn);
			case ARCH_arm64:
				return Arch::arm64::PatchFinder::step64(macho, start, length, is_ins, Rt, Rn);
			default:
				break;
		}

		return 0;
	}

	mach_vm_address_t stepBack64(MachO *macho, mach_vm_address_t start, size_t length, bool (*is_ins)(uint32_t*), int Rt, int Rn)
	{
		enum Architectures architecture = Arch::getCurrentArchitecture();

		switch(architecture)
		{
			case ARCH_x86_64:
				return Arch::x86_64::PatchFinder::stepBack64(macho, start, length, is_ins, Rt, Rn);
			case ARCH_arm64:
				return Arch::arm64::PatchFinder::stepBack64(macho, start, length, is_ins, Rt, Rn);
			default:
				break;
		}

		return 0;
	}

	mach_vm_address_t findFunctionBegin(MachO *macho, mach_vm_address_t start, mach_vm_address_t where)
	{
		enum Architectures architecture = Arch::getCurrentArchitecture();

		switch(architecture)
		{
			case ARCH_x86_64:
				return Arch::x86_64::PatchFinder::findFunctionBegin(macho, start, where);
			case ARCH_arm64:
				return Arch::arm64::PatchFinder::findFunctionBegin(macho, start, where);
			default:
				break;
		}

		return 0;
	}

	mach_vm_address_t findReference(MachO *macho, mach_vm_address_t to, int n, enum text which_text)
	{
		enum Architectures architecture = Arch::getCurrentArchitecture();

		switch(architecture)
		{
			case ARCH_x86_64:
				return Arch::x86_64::PatchFinder::findReference(macho, to, n, which_text);
			case ARCH_arm64:
				return Arch::arm64::PatchFinder::findReference(macho, to, n, which_text);
			default:
				break;
		}

		return 0;
	}

	mach_vm_address_t findDataReference(MachO *macho, mach_vm_address_t to, enum data which_data, int n)
	{
		enum Architectures architecture = Arch::getCurrentArchitecture();

		switch(architecture)
		{
			case ARCH_x86_64:
				return Arch::x86_64::PatchFinder::findDataReference(macho, to, which_data, n);
			case ARCH_arm64:
				return Arch::arm64::PatchFinder::findDataReference(macho, to, which_data, n);
			default:
				break;
		}

		return 0;
	}

	uint8_t* findString(MachO *macho, char *string, mach_vm_address_t base, mach_vm_address_t size, bool full_match)
	{
		enum Architectures architecture = Arch::getCurrentArchitecture();

		switch(architecture)
		{
			case ARCH_x86_64:
				return Arch::x86_64::PatchFinder::findString(macho, string, base, size, full_match);
			case ARCH_arm64:
				return Arch::arm64::PatchFinder::findString(macho, string, base, size, full_match);
			default:
				break;
		}

		return 0;
	}

	mach_vm_address_t findStringReference(MachO *macho, char *string, int n, enum string which_string, enum text which_text, bool full_match)
	{
		enum Architectures architecture = Arch::getCurrentArchitecture();

		switch(architecture)
		{
			case ARCH_x86_64:
				return Arch::x86_64::PatchFinder::findStringReference(macho, string, in, which_string, which_text, full_match);
			case ARCH_arm64:
				return Arch::arm64::PatchFinder::findStringReference(macho, string, in, which_string, which_text, full_match);
			default:
				break;
		}

		return 0;
	}

	void printInstruction64(MachO *macho, mach_vm_address_t start, uint32_t length, bool (*is_ins)(uint32_t*), int Rt, int Rn)
	{
	}

}