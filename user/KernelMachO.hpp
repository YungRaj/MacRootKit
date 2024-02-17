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

namespace xnu
{
	class Kernel;

	class KernelMachO : public MachO 
	{
		public:
			KernelMachO() { }

			KernelMachO(uintptr_t address);
			KernelMachO(uintptr_t address, off_t slide);

			KernelMachO(const char *path, off_t slide);
			KernelMachO(const char *path);

			~KernelMachO();

			virtual void parseLinkedit();

			virtual bool parseLoadCommands();

			virtual void parseMachO();

		private:
			xnu::Kernel *kernel;

		protected:
			uint8_t *linkedit;

			mach_vm_address_t linkedit_off;
			
			size_t linkedit_size;
	};

	class KernelCacheMachO : public KernelMachO
	{
		public:
			KernelCacheMachO(mach_vm_address_t kc, uintptr_t address);
			KernelCacheMachO(mach_vm_address_t kc, uintptr_t address, off_t slide);

			virtual bool parseLoadCommands();

		private:
			mach_vm_address_t kernel_cache;
	};

}
