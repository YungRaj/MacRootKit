#include "Backtrace.hpp"

#include "MacRootKit.hpp"

#include "Kernel.hpp"
#include "Kext.hpp"

#include "MachO.hpp"

#include "Segment.hpp"
#include "Section.hpp"

#include "Symbol.hpp"
#include "SymbolTable.hpp"

void Debug::Symbolicate::lookForAddressInsideKexts(xnu::Mach::VmAddress address, std::vector<xnu::Kext*> &kexts, Symbol *&sym)
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
					Offset delta = address - sym->getAddress();

					Offset new_delta = address - symbol->getAddress();

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

void Debug::Symbolicate::lookForAddressInsideKernel(xnu::Mach::VmAddress address, xnu::Kernel *kernel, Symbol *&sym)
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
					Offset delta = address - sym->getAddress();

					Offset new_delta = address - symbol->getAddress();

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

Symbol* Debug::Symbolicate::getSymbolFromAddress(xnu::Mach::VmAddress address, Offset *delta)
{
	Symbol *sym = NULL;

	mrk::MacRootKit *mrk = mac_rootkit_get_rootkit();

	xnu::Kernel *kernel = mrk->getKernel();

	std::vector<xnu::Kext*> &kexts = mrk->getKexts();

	Debug::Symbolicate::lookForAddressInsideKernel(address, kernel, sym);

	Debug::Symbolicate::lookForAddressInsideKexts(address, kexts, sym);

	return sym;
}

void Debug::printBacktrace(union Arch::ThreadState *thread_state)
{
	constexpr Arch::Architectures arch = Arch::getCurrentArchitecture();

	if constexpr(Arch::_arm64<arch>)
	{
		UInt64 fp = thread_state->state_arm64.fp;

		UInt32 frame = 0;

		while(fp)
		{
			Symbol *symbol;

			Offset delta;

			UInt64 lr = *(UInt64*) (fp - sizeof(UInt64));

			symbol = Debug::Symbolicate::getSymbolFromAddress(lr, &delta);

			MAC_RK_LOG("frame %u: 0x%x %s + %llu", frame, lr, symbol->getName(), delta);

			fp = *(UInt64*) fp;
		}
	}

	if constexpr(Arch::_x86_64<arch>)
	{
		UInt64 rbp = thread_state->state_x86_64.rbp;

		UInt32 frame = 0;

		while(rbp)
		{
			Symbol *symbol;

			Offset delta;

			UInt64 rip = *(UInt64*) (rbp - sizeof(UInt64));

			symbol = Debug::Symbolicate::getSymbolFromAddress(rip, &delta);

			MAC_RK_LOG("frame %u: 0x%x %s + %llu", frame, rip, symbol->getName(), delta);

			rbp = *(UInt64*) rbp;

			frame++;
		}
	}
}