#include "Isa_arm64.hpp"

/*
void push_registers_arm64() {}
void push_registers_arm64_end() {}

void set_argument_arm64() {}
void set_argument_arm64_end() {}

void check_breakpoint_arm64() {}
void check_breakpoint_arm64_end() {}

void breakpoint_arm64() {}
void breakpoint_arm64_end() {}

void pop_registers_arm64() {}
void pop_registers_arm64_end() {}
*/

namespace Arch
{
	namespace arm64
	{
		size_t BreakpointSize() { return Breakpoint; }

		union Breakpoint makeBreakpoint()
		{
			union Breakpoint breakpoint;

			breakpoint.brk.op = Arch::arm64::BreakpointPrefix;
			breakpoint.brk.imm = 0b0;
			breakpoint.brk.z = 0b0;

			return breakpoint;
		}

		size_t NormalBranchSize() { return NormalBranch; }
		size_t IndirectBranchSize() { return IndirectBranch; }

		union Branch makeBranch(mach_vm_address_t to, mach_vm_address_t from)
		{
			union Branch branch;

			bool sign;

			mach_vm_address_t imm;

			mach_vm_address_t max;
			mach_vm_address_t min;

			max = (from > to) ? from : to;
			min = (from > to) ? to   : from;

			mach_vm_address_t diff = (max - min);

			if(from > to)
			{
				sign = true;
			} else
			{
				sign = false;
			}

			branch.branch.mode = 0b0;
			branch.branch.op = Arch::arm64::NormalBranchPrefix;

			imm = diff;

			imm >>= 2;

			if(sign)
				imm = (~(imm - 1) & 0x1FFFFFF) | 0x2000000;

			branch.branch.imm = static_cast<uint32_t>(imm);

			return branch;
		}

		size_t FunctionCallSize() { return CallFunction; }

		union FunctionCall makeCall(mach_vm_address_t to, mach_vm_address_t from)
		{
			union FunctionCall call;

			bool sign;

			mach_vm_address_t imm;

			mach_vm_address_t max;
			mach_vm_address_t min;

			uint32_t insn_length = sizeof(uint32_t);

			from = from + insn_length;

			max = (from > to) ? from : to;
			min = (from > to) ? to   : from;

			mach_vm_address_t diff = (max - min);

			if(from > to)
			{
				sign = true;
			} else 
			{
				sign = false;
			}

			call.c.mode = 0b1;
			call.c.op = Arch::arm64::CallFunctionPrefix;

			imm >>= 2;

			if(sign)
				imm = (~imm + 1) | 0x2000000;

			call.c.imm = static_cast<uint32_t>(imm);

			return call;
		}
	}
}