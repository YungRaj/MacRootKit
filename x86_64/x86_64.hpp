#ifndef __X86_64_HPP_
#define __X86_64_HPP_

#include "Arch.hpp"

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