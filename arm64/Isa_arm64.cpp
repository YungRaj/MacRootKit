#include "Isa_arm64.hpp"

namespace Arch
{
	namespace arm64
	{
		union Breakpoint makeBreakpoint()
		{
			union Breakpoint breakpoint;

			breakpoint.brk.op = Arch::arm64::BreakpointPrefix;
			breakpoint.brk.imm = 0b0;
			breakpoint.brk.Z = 0b0;

			return breakpoint;
		}

		union Branch makeBranch(mach_vm_address_t to, mach_vm_address_t from)
		{
			union Branch branch;

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

			branch.branch.mode = 0b0;
			branch.branch.op = Arch::arm64::NormalBranchPrefix;

			imm >>= 2;

			if(sign)
				imm = (~imm + 1) | 0x2000000;

			branch.branch.imm = static_cast<uint32_t>(imm);

			return branch;
		}

		union FunctionCall makeCall(mach_vm_address_t to, mach_vm_address_t from);
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