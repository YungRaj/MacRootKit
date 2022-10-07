#ifndef __DISASSEMBLE_HPP_
#define __DISASSEMBLE_HPP_

class MachO;

namespace Arch
{
	namespace arm64
	{
		namespace Disassembler {}
	}
}

using namespace Arch::arm64;
using namespace Arch::arm64::Disassembler;

extern "C"
{
	char* condition(uint8_t c);
	char* shift(aarch64_shift s);
	char* ext(aarch64_extend e);
	char* reg(uint8_t reg, uint8_t sf);
	char* tlbi_op(uint8_t op1, uint8_t CRm, uint8_t op2);
};

namespace Arch
{
	namespace arm64
	{
		namespace Disassembler
		{
			bool disassemble(MachO *macho, mach_vm_address_t pc, uint32_t op);
			void disassemble(MachO *macho, mach_vm_address_t start, uint64_t *length);
		};
	};
};

#endif