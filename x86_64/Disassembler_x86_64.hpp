#ifndef __DISASSEMBLER_X86_64_HPP_
#define __DISASSEMBLER_X86_64_HPP_

#include <capstone/capstone.h>

#include "Array.hpp"

struct DisasmSig;

namespace Arch
{
	namespace x86_64
	{
		namespace Disassembler
		{
			bool init();

			bool deinit();

			size_t instructionSize(mach_vm_address_t address, size_t min);

			size_t quickInstructionSize(mach_vm_address_t address, size_t min);

			size_t disassemble(mach_vm_address_t address, size_t size, cs_insn **result);

			mach_vm_address_t disassembleNthCall(mach_vm_address_t address, size_t num, size_t lookup_size);

			mach_vm_address_t disassembleNthJmp(mach_vm_address_t address, size_t num, size_t lookup_size);

			mach_vm_address_t disassembleNthInstruction(mach_vm_address_t address, x86_insn insn, size_t num, size_t lookup_size);

			mach_vm_address_t disassembleSignature(mach_vm_address_t address, std::Array<struct DisasmSig*> *signature, size_t num, size_t lookup_size);
		}
	}
};

#endif