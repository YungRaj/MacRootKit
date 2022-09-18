#ifndef __ARCH_HPP_
#define __ARCH_HPP_

#include "APIUtil.hpp"

#include <x86_64/x86_64.hpp>
#include <arm64/arm64.hpp>

namespace Arch
{
	class Architecture;

	enum Architectures
	{
		ARCH_unknown 	= -1,
		ARCH_none 		=  0,
		ARCH_i386,
		ARCH_x86_64,
		ARCH_armv7,
		ARCH_arm64,
		ARCH_PowerPC,
		ARCH_SPARC,
		ARCH_MIPS
	};

	extern enum Architectures current_architecture;

	Architecture* initArchitecture();

	enum Architectures getCurrentArchitecture();

	extern "C"
	{
		void push_registers_x86_64();
		void push_registers_x86_64_end();
		void set_argument_x86_64();
		void set_argument_x86_64_end();
		void check_breakpoint_x86_64();
		void check_breakpoint_x86_64_end();
		void breakpoint_x86_64();
		void breakpoint_x86_64_end();
		void pop_registers_x86_64();
		void pop_registers_x86_64_end();

		void push_registers_arm64();
		void push_registers_arm64_end();
		void set_argument_arm64();
		void set_argument_arm64_end();
		void check_breakpoint_arm64();
		void check_breakpoint_arm64_end();
		void breakpoint_arm64();
		void breakpoint_arm64_end();
		void pop_registers_arm64();
		void pop_registers_arm64_end();
	}

#ifdef __x86_64__

	#define push_registers            push_registers_x86_64
	#define push_registers_end        push_registers_x86_64_end

	#define set_argument              set_argument_x86_64
	#define set_argument_end          set_argument_x86_64_end

	#define check_breakpoint          check_breakpoint_x86_64
	#define check_breakpoint_end      check_breakpoint_x86_64_end

	#define breakpoint                breakpoint_x86_64
	#define breakpoint_end            breakpoint_x86_64_end

	#define pop_registers             pop_registers_x86_64
	#define pop_registers_end         pop_registers_x86_64_end

#elif __arm64__

	extern "C"
	{
		void push_registers_arm64();
		void push_registers_arm64_end();

		void set_argument_arm64();
		void set_argument_arm64_end();

		void check_breakpoint_arm64();
		void check_breakpoint_arm64_end();

		void breakpoint_arm64();
		void breakpoint_arm64_end();

		void pop_registers_arm64();
		void pop_registers_arm64_end();
	}

	#define push_registers            push_registers_arm64
	#define push_registers_end        push_registers_arm64_end

	#define set_argument              set_argument_arm64
	#define set_argument_end          set_argument_arm64_end

	#define check_breakpoint          check_breakpoint_arm64
	#define check_breakpoint_end      check_breakpoint_arm64_end

	#define breakpoint                breakpoint_arm64
	#define breakpoint_end            breakpoint_arm64_end

	#define pop_registers             pop_registers_arm64
	#define pop_registers_end         pop_registers_arm64_end

#endif

	using RegisterState_x86_64 = struct Arch::x86_64::x86_64_register_state;
	using RegisterState_arm64 = struct Arch::arm64::arm64_register_state;

	using FunctionJmp_x86_64 = union Arch::x86_64::Jump;
	using FunctionJmp_arm64 = union Arch::arm64::Branch;

	using FunctionCall_x86_64 = union Arch::x86_64::FunctionCall;
	using FunctionCall_arm64 = union Arch::arm64::FunctionCall;

	using Breakpoint_x86_64 = union Arch::x86_64::Breakpoint;
	using Breakpoint_arm64 = union Arch::arm64::Breakpoint;

	union RegisterState
	{
		RegisterState_x86_64 state_x86_64;
		RegisterState_arm64 state_arm64;
	};

	union FunctionJmp
	{
		FunctionJmp_x86_64 jmp_x86_64;
		FunctionJmp_arm64  jmp_arm64;
	};

	union FunctionCall
	{
		FunctionCall_x86_64 call_x86_64;
		FunctionCall_arm64 call_arm64;
	};

	union Breakpoint
	{
		Breakpoint_x86_64 breakpoint_x86_64;
		Breakpoint_arm64 breakpoint_arm64;
	};

	union ThreadState
	{
		x86_thread_state64_t state_x86_64;
		arm_thread_state64_t state_arm64;
	};

	class Architecture
	{
		public:
			Architecture();

			~Architecture();

			static enum Architectures getArchitecture();

			size_t getBranchSize();

			size_t getCallSize();

			size_t getBreakpointSize();

			bool setInterrupts(bool enable);

			bool setWPBit(bool enable);

			bool setNXBit(bool enable);

			bool setPaging(bool enable);

			void makeJmp(union FunctionJmp *patch, mach_vm_address_t to, mach_vm_address_t from);

			void makeCall(union FunctionCall *call, mach_vm_address_t to, mach_vm_address_t from);

			void makeBreakpoint(union Breakpoint *breakpoint);

		private:
			enum Architectures arch;
	};

	namespace i386 { };
	namespace x86_64 { };

	namespace armv7 { };
	namespace arm64 { };

	namespace PowerPC { };

	namespace SPARC { };

	namespace MIPS { };
};

#endif