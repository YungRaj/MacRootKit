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

#include <mach/mach_types.h>
#include <mach/vm_types.h>

#include "Hook.hpp"

#include "Arch.hpp"

namespace xnu
{
	class Kernel;
	class Task;
};

using namespace xnu;

namespace mrk
{
	class Payload
	{
		static constexpr uint32_t expectedSize = Arch::getPageSize<Arch::getCurrentArchitecture()>();

		public:
			Payload(xnu::Task *task, Hook *hook, vm_prot_t prot);

			~Payload();

			Hook* getHook() { return hook; }

			mach_vm_address_t getAddress() { return address; }

			void setCurrentOffset(off_t offset);

			off_t getCurrentOffset() { return current_offset; }

			size_t getSize() { return size; }

			vm_prot_t getProt() { return prot; }

			Task* getTask() { return task; }

			bool readBytes(uint8_t *bytes, size_t size);
			bool readBytes(off_t offset, uint8_t *bytes, size_t size);

			bool writeBytes(uint8_t *bytes, size_t size);
			bool writeBytes(off_t offset, uint8_t *bytes, size_t size);

			void setWritable();
			void setExecutable();

			bool prepare();

			bool commit();
		
		private:
			xnu::Task *task;

			mach_vm_address_t address;

			off_t current_offset;

			Hook *hook;

			bool kernelPayload = false;

			size_t size;

			vm_prot_t prot;
	};
};
