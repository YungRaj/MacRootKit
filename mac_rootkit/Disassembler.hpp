#ifndef __DISASSEMBLER_HPP_
#define __DISASSEMBLER_HPP_

#include <capstone/capstone.h>

#include "Arch.hpp"
#include "Task.hpp"

using namespace Arch;

struct DisasmSig
{
	union
	{
		x86_insn insn_x86_64;

		arm64_insn insn_arm64;
	} insn;

	bool sub;
	bool addr;

	static DisasmSig* create()
	{
		return new DisasmSig;
	}

	static void deleter(DisasmSig *sig)
	{
		delete sig;
	}
};

enum DisassemblerType
{
	DisassemblerType_x86_64,
	DisassemblerType_arm64,
	DisassemblerType_Unknown,
	DisassemblerType_None,
};

class Disassembler
{
	public:
		Disassembler(xnu::Task *task);

		~Disassembler();

		enum Architectures getArchitecture() { return architecture; }

		enum DisassemblerType getDisassemblerType() { return disassembler; }

		void initDisassembler();

		void deinitDisassembler();

		size_t disassemble(mach_vm_address_t address, size_t size, cs_insn **result);

		size_t quickInstructionSize(mach_vm_address_t address, size_t min);

		size_t instructionSize(mach_vm_address_t address, size_t min);

		mach_vm_address_t disassembleNthCall(mach_vm_address_t address, size_t num, size_t lookup_size);

		mach_vm_address_t disassembleNthJmp(mach_vm_address_t address, size_t num, size_t lookup_size);

		mach_vm_address_t disassembleNthInstruction(mach_vm_address_t address, uint32_t insn, size_t num, size_t lookup_size);

		mach_vm_address_t disassembleSignature(mach_vm_address_t address, std::Array<struct DisasmSig*> *signature, size_t num, size_t lookup_size);

	private:
		enum Architectures architecture;

		enum DisassemblerType disassembler;

		xnu::Task *task;

		enum DisassemblerType getDisassemblerFromArch();
};

#endif