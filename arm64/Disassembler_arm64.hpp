#ifndef __DISASSEMBLER_ARM64_HPP_
#define __DISASSEMBLER_ARM64_HPP_

#include <capstone/capstone.h>

#include "Array.hpp"

struct DisasmSig;

namespace Arch
{
	namespace arm64
	{
		namespace Disassembler
		{
			bool init();

			bool deinit();

			size_t instructionSize(mach_vm_address_t address, size_t min);

			size_t quickInstructionSize(mach_vm_address_t address, size_t min);

			size_t disassemble(mach_vm_address_t address, size_t size, cs_insn **result);

			bool registerAccess(cs_insn *insn, cs_regs regs_read, uint8_t *nread, cs_regs regs_write, uint8_t *nwrite);

			mach_vm_address_t disassembleNthBranchLink(mach_vm_address_t address, size_t num, size_t lookup_size);

			mach_vm_address_t disassembleNthBranch(mach_vm_address_t address, size_t num, size_t lookup_size);

			mach_vm_address_t disassembleNthInstruction(mach_vm_address_t address, arm64_insn insn, size_t num, size_t lookup_size);

			mach_vm_address_t disassembleSignature(mach_vm_address_t address, std::Array<struct DisasmSig*> *signature, size_t num, size_t lookup_size);
		}
	}
};

#endif