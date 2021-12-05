#ifndef __DISASSEMBLER_HPP_
#define __DISASSEMBLER_HPP_

#include "Arch.hpp"

#include <x86_64/Disassembler_x86_64.hpp>
#include <arm64/Disassembler_arm64.hpp>

using namespace Arch;

class Disassembler
{
	public:
		Disassembler(Task *task);

		enum Architectures getArchitecture() { return architecture; }

		void initDisassembler();

		void deinitDisassembler();

		size_t disassemble(mach_vm_address_t address, size_t size, cs_insn **result);

		size_t quickInstructionSize(mach_vm_address_t address, size_t min);

		size_t instructionSize(mach_vm_address_t address, size_t min);

		mach_vm_address_t disassembleNthCall(mach_vm_address_t address, size_t num, size_t lookup_size);

		mach_vm_address_t disassembleNthJmp(mach_vm_address_t address, size_t num, size_t lookup_size);

		mach_vm_address_t disassembleNthInstruction(mach_vm_address_t, size_t num, size_t lookup_size);

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

		mach_vm_address_t disassembleSignature(mach_vm_address_t address, Array<DisasmSig*> *signature, size_t num, size_t lookup_size);

	private:
		enum Architectures architecture;

		Task *task;
};

#endif