#include "Isa_x86_64.hpp"

namespace Arch
{
	namespace x86_64
	{
		union Breakpoint makeBreakpoint()
		{
			union Breakpoint breakpoint;

			breakpoint.intN.intN = Arch::x86_64::BreakpointPrefix;

			return breakpoint;
		}

		union Jump makeJump(mach_vm_address_t to, mach_vm_address_t from)
		{
			union Jump jmp;

			uint32_t insn_length = sizeof(uint8_t) + sizeof(uint32_t);

			int32_t imm;

			mach_vm_address_t max;
			mach_vm_address_t min;

			from = from + insn_length;

			max = (from > to) ? from : to;
			min = (from > to) ? to   : from;

			mach_vm_address_t diff = (max - min);

			if(diff > INT32_MAX)
			{
				MAC_RK_LOG("MacRK::Arch::x86_64::makeJump() encoding too large!\n");
			}

			imm = static_cast<int32_t>(diff);

			if(from > to)
				imm *= -1;

			jmp.s.opcode = Arch::x86_64::SmallJumpPrefix;
			jmp.s.argument = imm;

			return jmp;
		}

		union FunctionCall makeCall(mach_vm_address_t to, mach_vm_address_t from)
		{
			union FunctionCall call;

			uint32_t insn_length = sizeof(uint8_t) + sizeof(uint32_t);

			int32_t imm;

			mach_vm_address_t max;
			mach_vm_address_t min;

			from = from + insn_length;

			max = (from > to) ? from : to;
			min = (from > to) ? to   : from;

			mach_vm_address_t diff = (max - min);

			if(diff > INT32_MAX)
			{
				MAC_RK_LOG("MacRK::Arch::x86_64::makeCall() encoding too large!\n");
			}

			imm = static_cast<int32_t>(diff);

			if(from > to)
				imm *= -1;

			call.c.opcode = Arch::x86_64::FunctionCallPrefix;
			call.c.argument = imm;

			return call;
		}
	}
}