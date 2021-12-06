#include "Arch.hpp"

namespace Arch
{
	enum Architectures current_architecture = ARCH_unknown;

	enum Architectures getArchitecture()
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

	bool setInterrupts(bool enable)
	{
		bool success = false;

		switch(current_architecture)
		{
			case ARCH_x86_64:
				success = Arch::x86_64::setInterrupts(enable);

				break;
			case ARCH_arm64:
				success = Arch::arm64::setInterrupts(enable);

				break;
			default:
				break;
		}

		return success;
	}

	bool setWPBit(bool enable)
	{
		bool success = false;

		switch(current_architecture)
		{
			case ARCH_x86_64:
				success = Arch::x86_64::setWPBit(enable);

				break;
			case ARCH_arm64:
				success = Arch::arm64::setWPBit(enable);

				break;
			default:
				break;
		}

		return success;
	}

	bool setNXBit(bool enable)
	{
		bool success = false;

		switch(current_architecture)
		{
			case ARCH_x86_64:
				success = Arch::x86_64::setNXBit(enable);

				break;
			case ARCH_arm64:
				success = Arch::arm64::setNXBit(enable);

				break;
			default:
				break;
		}

		return success;
	}

	bool setPaging(bool enable)
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
}