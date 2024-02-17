/*
 * Copyright (c) YungRaj
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

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

	void printBacktrace(union Arch::ThreadState *thread_state);
};
