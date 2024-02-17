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

#include <mach/mach_types.h>
#include <mach/kmod.h>

#include <Types.h>

#include "MachO.hpp"

#include "vector.hpp"
#include "SymbolTable.hpp"

namespace xnu
{
	class Kernel;
	class Kext;
	class KextMachO;
}

namespace xnu
{
	class KextMachO : public MachO
	{
		public:
			KextMachO(xnu::Kernel *kernel, char *name, xnu::Mach::VmAddress base);
			KextMachO(xnu::Kernel *kernel, char *name, kmod_info_t *kmod_info);

			~KextMachO();

			xnu::Kernel* getKernel() { return kernel; }

			char* getKextName() { return name; }

			xnu::Mach::VmAddress getAddress() { return address; }

			virtual Size getSize() { return kmod_info->size > 0 ? kmod_info->size : MachO::getSize(); }

			kmod_start_func_t* getKmodStart() { return kmod_info->start; }
			kmod_stop_func_t* getKmodStop() { return kmod_info->stop; }

			void setKernelCollection(xnu::Mach::VmAddress kc) { this->kernel_collection = kc; }

			virtual void parseLinkedit();

			virtual bool parseLoadCommands();

			virtual void parseHeader();

			virtual void parseMachO();

		private:
			xnu::Kernel *kernel;

			xnu::Mach::VmAddress address;

			char *name;

			Offset base_offset;

			xnu::Mach::VmAddress kernel_cache;
			xnu::Mach::VmAddress kernel_collection;

			kmod_info_t *kmod_info;

			UInt8 *linkedit;

			Offset linkedit_off;

			Size linkedit_size;
	};
}
