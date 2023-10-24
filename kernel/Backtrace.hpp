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

namespace xnu
{
	class Kext;
	class Kernel;
}

mrk::MacRootKit* mac_rootkit_get_rootkit();

namespace Debug
{
	namespace Symbolicate
	{
		void lookForAddressInsideKernel(mach_vm_address_t address, xnu::Kernel *kernel, Symbol *&sym);
		void lookForAddressInsideKexts(mach_vm_address_t address, std::vector<xnu::Kext*> &kexts, Symbol *&sym);

		Symbol* getSymbolFromAddress(mach_vm_address_t address, off_t *delta);
	};

	void printBacktrace(union ThreadState *thread_state);
};

#endif