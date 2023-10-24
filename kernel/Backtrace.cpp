#include "Backtrace.hpp"

#include "MacRootKit.hpp"

#include "Kernel.hpp"
#include "Kext.hpp"

#include "MachO.hpp"

#include "Segment.hpp"
#include "Section.hpp"

#include "Symbol.hpp"
#include "SymbolTable.hpp"

void Debug::Symbolicate::lookForAddressInsideKexts(mach_vm_address_t address, std::vector<xnu::Kext*> &kexts, Symbol *&sym)
{
	for(int i = 0; i < kexts.size(); i++)
	{
		xnu::Kext *kext = kexts.at(i);

		std::vector<Symbol*> &kextSymbols = kext->getAllSymbols();

		for(int j = 0; j < kextSymbols.size(); i++)
		{
			Symbol *symbol = kextSymbols.at(j);

			if(symbol->getAddress() > address)
			{
				if(sym)
				{
					off_t delta = sym->getAddress() - address;

					off_t new_delta = symbol->getAddress() - address;

					if(new_delta < delta)
					{
						if(new_delta <= Arch::getPageSize<Arch::getCurrentArchitecture()>() * 3)
						{
							sym = symbol;
						}
					}

				} else
				{
					sym = symbol;
				}
			}
		}
	}
}

void Debug::Symbolicate::lookForAddressInsideKernel(mach_vm_address_t address, xnu::Kernel *kernel, Symbol *&sym)
{
	std::vector<Symbol*> &kernelSymbols = kernel->getAllSymbols();

	for(int i = 0; i < kernelSymbols.size(); i++)
	{
		Symbol *symbol = kernelSymbols.at(i);

		if(symbol->getAddress() > address)
		{
			if(sym)
			{
				off_t delta = sym->getAddress() - address;

				off_t new_delta = symbol->getAddress() - address;

				if(new_delta < delta)
				{
					if(new_delta <= Arch::getPageSize<Arch::getCurrentArchitecture()>() * 3)
					{
						sym = symbol;
					}
				}

			} else
			{
				sym = symbol;
			}
		}
	}
}

Symbol* Debug::Symbolicate::getSymbolFromAddress(mach_vm_address_t address, off_t *delta)
{
	Symbol *sym = NULL;

	mrk::MacRootKit *mrk = mac_rootkit_get_rootkit();

	xnu::Kernel *kernel = mrk->getKernel();

	std::vector<xnu::Kext*> &kexts = mrk->getKexts();

	Debug::Symbolicate::lookForAddressInsideKernel(address, kernel, sym);

	Debug::Symbolicate::lookForAddressInsideKexts(address, kexts, sym);

	return sym;
}

void Debug::printBacktrace(union ThreadState *thread_state)
{

}