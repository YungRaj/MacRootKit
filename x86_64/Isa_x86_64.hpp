#ifndef __ISA_X86_64_HPP_
#define __ISA_X86_64_HPP_

#include "Arch.hpp"

namespace Arch
{
	namespace x86_64
	{
		struct x86_64_register_state
		{
			uint64_t rsp;
			uint64_t rbp;
			uint64_t rax;
			uint64_t rbx;
			uint64_t rcx;
			uint64_t rdx;
			uint64_t rdi;
			uint64_t rsi;
			uint64_t r8;
			uint64_t r9;
			uint64_t r10;
			uint64_t r11;
			uint64_t r12;
			uint64_t r13;
			uint64_t r14;
			uint64_t r15;
		};

		static constexpr uint16_t Breakpoint = sizeof(uint16_t);
		static constexpr uint16_t BreakpointPrefix = 0x03CD;

		union Breakpoint
		{
			struct PACKED Int3
			{
				uint8_t int3;
			} int3;

			struct PACKED IntN
			{
				uint16_t intN;
			} intN;
		};

		static constexpr size_t SmallJump = 1 + sizeof(int32_t);
		static constexpr size_t LongJump = 6 + sizeof(uintptr_t);
		static constexpr size_t NearJump = 6;

		static constexpr uint8_t = SmalJumpPrefix = 0xE9;
		static constexpr uint16_t LongJumpPrefix = 0x25FF;

		enum class JumpType
		{
			Auto,
			Long,
			Short,
			Near,
		};

		union FunctionPatch
		{
			struct PACKED LongPatch
			{
				uint16_t opcode;
				int32_t argument;
				uintptr_t disp;
				uint8_t org[sizeof(uint64_t) - sizeof(uintptr_t) + sizeof(uint16_t)];
			} l;

			struct PACKED NearPatch
			{
				uint16_t opcode;
				int32_t argument;
				uintptr_t disp;
				uint8_t org[2];
			} n;

			struct PACKED ShortPatch
			{
				uint8_t opcode;
				int32_t argument;
				uint8_t org[3];
			} s;

			uint64_t value;
		};

		static constexpr size_t FunctionCall = sizeof(uint8_t) + sizeof(int32_t);
		static constexpr uint8_t FunctionCallPrefix = 0xE8;

		union FunctionCall
		{
			struct PACKED CallFunction
			{
				uint8_t opcode;
				int32_t argument;
				uint8_t org[3];
			} c;

			uint64_t value;
		};
	};
};

#endif