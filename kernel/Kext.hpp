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

#include <stddef.h>
#include <stdint.h>

#include <mach/kmod.h>
#include <mach/mach_types.h>

#include "KextMachO.hpp"

namespace xnu {}

namespace xnu
{
	class Kernel;

	class Kext
	{
		public:
			Kext(xnu::Kernel *kernel, mach_vm_address_t base, char *identifier);

			Kext(xnu::Kernel *kernel, void *kext, kmod_info_t *kmod_info);

			~Kext();

			static xnu::Kext* findKextWithIdentifier(xnu::Kernel *kernel, char *name);
			static xnu::Kext* findKextWithIdentifier_deprecated(xnu::Kernel *kernel, char *name);

			static xnu::Kext* findKextWithId(xnu::Kernel *kernel, uint32_t kext_id);

			static void onKextLoad(void *kext, kmod_info_t *kmod_info);

			char* getName() { return identifier; }

			xnu::KextMachO* getMachO() { return macho; }

			mach_vm_address_t getBase() { return address; }

			mach_vm_address_t getAddress() { return address; }

			size_t getSize() { return size; }

			void* getOSKext() { return kext; }

			kmod_info_t* getKmodInfo() { return kmod_info; }

			kmod_start_func_t* getKmodStart() { return kmod_info->start; }
			kmod_stop_func_t*  getKmodStop() { return kmod_info->stop; }

			std::vector<Symbol*>& getAllSymbols() { return macho->getAllSymbols(); }

			Symbol* getSymbolByName(char *symname) { return macho->getSymbolByName(symname); }
			Symbol* getSymbolByAddress(mach_vm_address_t address) { return macho->getSymbolByAddress(address); }

			mach_vm_address_t getSymbolAddressByName(char *symbolname) { return macho->getSymbolAddressByName(symbolname); }

		private:
			xnu::Kernel *kernel;

			xnu::KextMachO *macho;

			kmod_info_t *kmod_info;

			void *kext;

			mach_vm_address_t address;

			size_t size;

			char *identifier;
	};

};

