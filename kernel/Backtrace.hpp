#ifndef __BACKTRACE_HPP_
#define __BACKTRACE_HPP_

#include "Arch.hpp"

class MachO;

class Segment;
class Section;

class Symbol;

namespace mrk
{
	class MacRootKit;
};

mrk::MacRootKit* mac_rootkit_get_rootkit();

namespace Debug
{
	namespace Symbolicate
	{
		Symbol* getSymbolFromAddress(mach_vm_address_t address, off_t *delta);
	};

	void printBacktrace(union ThreadState *thread_state);
};

#endif