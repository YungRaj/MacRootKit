#ifndef __X86_64_HPP_
#define __X86_64_HPP_

#include "Isa_x86_64.hpp"
#include "Disassembler_x86_64.hpp"
#include "PatchFinder_x86_64.hpp"

namespace Arch
{
	namespace x86_64
	{
		bool setInterrupts(bool enable);

		bool setNXBit(bool enable);

		bool setWPBit(bool enable);
	}
};

#endif