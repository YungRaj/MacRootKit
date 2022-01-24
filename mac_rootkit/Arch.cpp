#include "Arch.hpp"

namespace Arch
{
	enum Architectures current_architecture = ARCH_unknown;

	Architecture* initArchitecture()
	{
		static Architecture *architecture = NULL;

		if(!architecture)
		{
			architecture = new Architecture();
		}

		return architecture;
	}

	enum Architectures getCurrentArchitecture()
	{
		if(current_architecture == ARCH_unknown)
		{
			Architecture::getArchitecture();
		}

		return current_architecture;
	}

	Architecture::Architecture()
	{
		this->arch = Arch::getCurrentArchitecture();
	}

	Architecture::~Architecture()
	{

	}

	enum Architectures Architecture::getArchitecture()
	{
		enum Architectures arch;

	#ifdef __x86_64__
		arch = ARCH_x86_64;
	#elif __arm64__
		arch = ARCH_arm64;
	#else
		arch = ARCH_unknown;
	#endif

		current_architecture = arch;

		return current_architecture;
	}

	bool Architecture::setInterrupts(bool enable)
	{
		bool success = false;

		switch(current_architecture)
		{
			case ARCH_x86_64:
				success = x86_64::setInterrupts(enable);

				break;
			case ARCH_arm64:
				success = arm64::setInterrupts(enable);

				break;
			default:
				break;
		}

		return success;
	}

	bool Architecture::setWPBit(bool enable)
	{
		bool success = false;

		switch(current_architecture)
		{
			case ARCH_x86_64:
				success = x86_64::setWPBit(enable);

				break;
			case ARCH_arm64:
				success = arm64::setWPBit(enable);

				break;
			default:
				break;
		}

		return success;
	}

	bool Architecture::setNXBit(bool enable)
	{
		bool success = false;

		switch(current_architecture)
		{
			case ARCH_x86_64:
				success = x86_64::setNXBit(enable);

				break;
			case ARCH_arm64:
				success = arm64::setNXBit(enable);

				break;
			default:
				break;
		}

		return success;
	}

	bool Architecture::setPaging(bool enable)
	{
		bool success = false;

		switch(current_architecture)
		{
			case ARCH_x86_64:
				break;
			case ARCH_arm64:
				break;
			default:
				break;
		}

		return success;
	}

	void Architecture::makeJmp(union FunctionJmp *jmp, mach_vm_address_t to, mach_vm_address_t from)
	{
		switch(current_architecture)
		{
			case ARCH_x86_64:
				jmp->jmp_x86_64 = x86_64::makeJump(to, from);

				break;
			case ARCH_arm64:
				jmp->jmp_arm64 = arm64::makeBranch(to, from);

				break;

			default:
				break;
		}
	}

	void Architecture::makeCall(union FunctionCall *call, mach_vm_address_t to, mach_vm_address_t from)
	{
		switch(current_architecture)
		{
			case ARCH_x86_64:
				call->call_x86_64 = x86_64::makeCall(to, from);

				break;
			case ARCH_arm64:
				call->call_arm64 = arm64::makeCall(to, from);

				break;

			default:
				break;
		}
	}

	void Architecture::makeBreakpoint(union Breakpoint *breakpoint)
	{
		switch(current_architecture)
		{
			case ARCH_x86_64:
				breakpoint->breakpoint_x86_64 = x86_64::makeBreakpoint();

				break;
			case ARCH_arm64:
				breakpoint->breakpoint_arm64 = arm64::makeBreakpoint();

				break;

			default:
				break;
		}
	}

	size_t Architecture::getBranchSize()
	{
		size_t branch_size = 0;

		switch(current_architecture)
		{
			case ARCH_x86_64:
				branch_size = x86_64::SmallJump;

				break;
			case ARCH_arm64:
				branch_size = arm64::NormalBranch;

				break;
			default:
				break;
		}

		return branch_size;
	}

	size_t Architecture::getCallSize()
	{
		size_t branch_size = 0;

		switch(current_architecture)
		{
			case ARCH_x86_64:
				branch_size = x86_64::FunctionCallSize();

				break;
			case ARCH_arm64:
				branch_size = arm64::FunctionCallSize();

				break;
			default:
				break;
		}

		return branch_size;
	}

	size_t Architecture::getBreakpointSize()
	{
		size_t breakpoint_size = 0;

		switch(current_architecture)
		{
			case ARCH_x86_64:
				breakpoint_size = x86_64::BreakpointSize();

				break;
			case ARCH_arm64:
				breakpoint_size = arm64::BreakpointSize();

				break;
			default:
				break;
		}

		return breakpoint_size;
	}
}