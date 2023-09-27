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
		ARCH_unsupported = -1,
		ARCH_none 		 =  0,
		ARCH_i386,
		ARCH_x86_64,
		ARCH_armv7,
		ARCH_arm64,
		ARCH_PowerPC,
		ARCH_SPARC,
		ARCH_MIPS
	};

	template <bool IsX86, bool IsArm, bool Is64Bit>
	struct CurrentArchitecture
	{
		static_assert((!IsX86 && IsArm) || (IsX86 && !IsArm), "Unsupported Architecture!");

	    static constexpr Architectures value = []
	    {
			if constexpr (IsX86)
			{
				if constexpr (Is64Bit)
					return ARCH_x86_64;
				else
					return ARCH_i386;

			} else if constexpr (IsArm)
			{
				if constexpr (Is64Bit)
					return ARCH_arm64;
				else
					return ARCH_armv7;
			} else
			{
				return ARCH_unsupported;
			}

		} ();
	};

	constexpr enum Architectures getCurrentArchitecture()
	{
	#ifdef __x86_64__
		return CurrentArchitecture<true, false, true>::value;
	#elif __arm64__
		return CurrentArchitecture<false, true, true>::value;
	#endif

		return ARCH_unsupported;
	}

	constexpr enum Architectures current_architecture = Arch::getCurrentArchitecture();

	Architecture* initArchitecture();

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

#elif __arm64__ || __arm64e__

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

	using Jmp_x86_64 = union Arch::x86_64::Jump;
	using Branch_arm64 = union Arch::arm64::Branch;

	using FunctionCall_x86_64 = union Arch::x86_64::FunctionCall;
	using FunctionCall_arm64 = union Arch::arm64::FunctionCall;

	using Breakpoint_x86_64 = union Arch::x86_64::Breakpoint;
	using Breakpoint_arm64 = union Arch::arm64::Breakpoint;

	union RegisterState
	{
		RegisterState_x86_64 state_x86_64;
		RegisterState_arm64 state_arm64;
	};

	union Branch
	{
		Jmp_x86_64 jmp_x86_64;
		Branch_arm64  br_arm64;
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
		} state_x86_64;

		struct arm64_register_state
		{
			uint64_t    x[29];
			uint64_t    fp;
			uint64_t    lr;
			uint64_t    sp;
			uint64_t    pc;
			uint64_t    cpsr;
		} state_arm64;
	};

	template<enum Architectures ArchType>
	concept _x86_64 = ArchType == ARCH_x86_64;

	template<enum Architectures ArchType>
	concept _arm64 = ArchType == ARCH_arm64;

	template<enum Architectures ArchType>
	concept _i386 = ArchType == ARCH_i386;

	template<enum Architectures ArchType>
	concept _armv7 = ArchType == ARCH_armv7;

	template<enum Architectures ArchType>
	concept SupportedProcessor = _x86_64<ArchType> || _arm64<ArchType> || _i386<ArchType> || _armv7<ArchType>;

	template <enum Architectures ArchType> requires SupportedProcessor<ArchType>
	class Instructions
	{
		public:

			constexpr static size_t getBranchSize()
			{
				if constexpr (ArchType == ARCH_x86_64)
				{
					return x86_64::SmallJumpSize();
				}
				else if constexpr (ArchType == ARCH_arm64)
				{
					return arm64::NormalBranchSize();
			    }
			}

			constexpr static size_t getCallSize()
			{
				if constexpr (ArchType == ARCH_x86_64)
				{
					return x86_64::FunctionCallSize();
				}
				else if constexpr (ArchType == ARCH_arm64)
				{
					return arm64::FunctionCallSize();
			    }
			}

			constexpr static size_t getBreakpointSize()
			{
				if constexpr (ArchType == ARCH_x86_64)
				{
					return x86_64::BreakpointSize();
				}
				else if constexpr (ArchType == ARCH_arm64)
				{
					return arm64::BreakpointSize();
			    }
			}

			static void makeBranch(union Branch *branch, mach_vm_address_t to, mach_vm_address_t from)
			{
				if constexpr (ArchType == ARCH_x86_64)
				{
					branch->jmp_x86_64 = x86_64::makeJump(to, from);
				}
				else if constexpr (ArchType == ARCH_arm64)
				{
					branch->br_arm64 = arm64::makeBranch(to, from);
			    }
			}

			static void makeCall(union FunctionCall *call, mach_vm_address_t to, mach_vm_address_t from)
			{
				if constexpr (ArchType == ARCH_x86_64)
				{
					call->call_x86_64 = x86_64::makeCall(to, from);
				}
				else if constexpr (ArchType == ARCH_arm64)
				{
					call->call_arm64 = arm64::makeCall(to, from);
			    }
			}

			static void makeBreakpoint(union Breakpoint *breakpoint)
			{
				if constexpr (ArchType == ARCH_x86_64)
				{
					breakpoint->breakpoint_x86_64 = x86_64::makeBreakpoint();
				}
				else if constexpr (ArchType == ARCH_arm64)
				{
					breakpoint->breakpoint_arm64 = arm64::makeBreakpoint();
			    }
			}
	};

	class Architecture
	{
		public:
			Architecture();

			~Architecture();

			static enum Architectures getArchitecture();

			constexpr size_t getBranchSize()
			{
				return Instructions<Arch::getCurrentArchitecture()>::getBranchSize();
			}

			constexpr size_t getCallSize()
			{
				return Instructions<Arch::getCurrentArchitecture()>::getCallSize();
			}

			constexpr size_t getBreakpointSize()
			{
				return Instructions<Arch::getCurrentArchitecture()>::getBreakpointSize();
			}

			bool setInterrupts(bool enable);

			bool setWPBit(bool enable);

			bool setNXBit(bool enable);

			bool setPaging(bool enable);

			void makeBranch(union Branch *branch, mach_vm_address_t to, mach_vm_address_t from)
			{
				return Instructions<Arch::getCurrentArchitecture()>::makeBranch(branch, to, from);
			}

			void makeCall(union FunctionCall *call, mach_vm_address_t to, mach_vm_address_t from)
			{
				return Instructions<Arch::getCurrentArchitecture()>::makeCall(call, to, from);
			}

			void makeBreakpoint(union Breakpoint *breakpoint)
			{
				return Instructions<Arch::getCurrentArchitecture()>::makeBreakpoint(breakpoint);
			}

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