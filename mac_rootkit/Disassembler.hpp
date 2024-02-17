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

#include <capstone/capstone.h>

#include "Arch.hpp"

namespace xnu
{
	class Kernel;
	class Task;
}

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

		mach_vm_address_t disassembleSignature(mach_vm_address_t address, std::vector<struct DisasmSig*> *signature, size_t num, size_t lookup_size);

	private:
		enum Architectures architecture;

		enum DisassemblerType disassembler;

		xnu::Task *task;

		enum DisassemblerType getDisassemblerFromArch();
};
