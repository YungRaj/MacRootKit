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

#include "MachO.hpp"

class MachO;

namespace xnu
{
	class Kernel;
	class Kext;

	class KernelMachO : public MachO 
	{
		public:
			KernelMachO() { }

			KernelMachO(xnu::Kernel *kernel);

			~KernelMachO();

			mach_vm_address_t getKernelCache() { return kernel_cache; }

			mach_vm_address_t getKernelCollection() { return kernel_collection; }

			void setKernelCache(mach_vm_address_t kc) { this->kernel_cache = kc; }

			void setKernelCollection(mach_vm_address_t kc) { this->kernel_collection = kc; }

			static xnu::Kext* kextLoadedAt(xnu::Kernel *kernel, mach_vm_address_t address);
			static xnu::Kext* kextWithIdentifier(xnu::Kernel *kernel, char *kext);

			virtual void parseLinkedit();

			virtual bool parseLoadCommands();

			virtual void parseMachO();

		protected:
			xnu::Kernel *kernel;

			mach_vm_address_t kernel_cache;

			mach_vm_address_t kernel_collection;

			uint8_t *linkedit;

			mach_vm_address_t linkedit_off;
			
			size_t linkedit_size;
	};

}
