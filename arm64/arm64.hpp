#ifndef __ARM64_HPP_
#define __ARM64_HPP_

#include "Arch.hpp"

#include "Isa_arm64.hpp"
#include "Disassembler_arm64.hpp"
#include "PatchFinder_arm64.hpp"

namespace Arch
{
	namespace arm64
	{
		bool setInterrupts(bool enable);

		bool setNXBit(bool enable);

		bool setWPBit(bool enable);

	};
};

#endif