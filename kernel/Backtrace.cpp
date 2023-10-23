#include "Backtrace.hpp"

#include "MachO.hpp"

#include "Segment.hpp"
#include "Section.hpp"

#include "Symbol.hpp"
#include "SymbolTable.hpp"

Symbol* Debug::Symbolicate::getSymbolFromAddress(mach_vm_address_t address, off_t *delta)
{
	return NULL;
}

void Debug::printBacktrace(union ThreadState *thread_state)
{

}