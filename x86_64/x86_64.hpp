#ifndef __X86_64_HPP_
#define __X86_64_HPP_

#include "Arch.hpp"

namespace Arch
{
	namespace x86_64
	{
		void setInterrupts(bool enable);

		void setNXBit(bool enable);

		void setWPBit(bool enable);
	}
};

#endif