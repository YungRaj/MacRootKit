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

#include "vector.hpp"

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

			bool registerAccess(cs_insn *insn, cs_regs regs_read, uint8_t *nread, cs_regs regs_write, uint8_t *nwrite);

			mach_vm_address_t disassembleNthCall(mach_vm_address_t address, size_t num, size_t lookup_size);

			mach_vm_address_t disassembleNthJmp(mach_vm_address_t address, size_t num, size_t lookup_size);

			mach_vm_address_t disassembleNthInstruction(mach_vm_address_t address, x86_insn insn, size_t num, size_t lookup_size);

			mach_vm_address_t disassembleSignature(mach_vm_address_t address, std::vector<struct DisasmSig*> *signature, size_t num, size_t lookup_size);
		}
	}
};
