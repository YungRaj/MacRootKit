#include "Arch.hpp"

namespace Arch
{
	Architecture* initArchitecture()
	{
		static Architecture *architecture = NULL;

		if(!architecture)
		{
			architecture = new Architecture();
		}

		return architecture;
	}


	Architecture::Architecture() : arch(Arch::getCurrentArchitecture())
	{
		
	}

	Architecture::~Architecture()
	{

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
}