#ifndef __ARCH_HPP_
#define __ARCH_HPP_

#include "APIUtil.hpp"

#include <x86_64/Arch_x86_64.hpp>
#include <arm64/Arch_arm64.hpp>

namespace Arch
{
	enum Architectures
	{
		ARCH_unknown 	= -1,
		ARCH_none 		= 0
		ARCH_i386,
		ARCH_x86_64,
		ARCH_armv7,
		ARCH_arm64,
		ARCH_PowerPC,
		ARCH_SPARC,
		ARCH_MIPS
	};

	extern enum Architectures current_architecture;

	bool setInterrupts(bool enable);

	bool setWPBit(bool enable);

	bool setNXBit(bool enable);

	namespace i386 { };
	namespace x86_64 { };

	namespace armv7 { };
	namespace arm64 { };

	namespace PowerPC { };

	namespace SPARC { };

	namespace MIPS { };
};

#endif