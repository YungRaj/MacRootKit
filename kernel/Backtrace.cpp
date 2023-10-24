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

		xnu::KextMachO *macho = kext->getMachO();

		if(!macho->addressInSegment(address, "__TEXT"))
			continue;

		std::vector<Symbol*> &kextSymbols = kext->getAllSymbols();

		for(int j = 0; j < kextSymbols.size(); i++)
		{
			Symbol *symbol = kextSymbols.at(j);

			if(address > symbol->getAddress())
			{
				if(sym)
				{
					off_t delta = address - sym->getAddress();

					off_t new_delta = address - symbol->getAddress();

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
	MachO *macho = kernel->getMachO();

	std::vector<Symbol*> &kernelSymbols = kernel->getAllSymbols();

	if(macho->addressInSegment(address, "__TEXT") ||
	   macho->addressInSegment(address, "__PRELINK_TEXT") ||
	   macho->addressInSegment(address, "__KLD"))
	{
		for(int i = 0; i < kernelSymbols.size(); i++)
		{
			Symbol *symbol = kernelSymbols.at(i);

			if(address > symbol->getAddress())
			{
				if(sym)
				{
					off_t delta = address - sym->getAddress();

					off_t new_delta = address - symbol->getAddress();

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