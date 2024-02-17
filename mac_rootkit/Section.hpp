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

extern "C"
{
	#include <mach-o.h>
	
	#include <mach/mach_types.h>
	#include <sys/types.h>

	#include <string.h>
}

#include "Log.hpp"

class Section
{
	public:
		Section(struct section_64 *section)
			: section(section),
			  address(section->addr),
			  offset(section->offset),
			  size(section->size)
		{
			name = new char[strlen(section->sectname) + 1];

			strlcpy(this->name, section->sectname, strlen(section->sectname) + 1);
		}

		~Section()
		{
			delete name;
		}

		struct section_64* getSection() { return section; }

		char* getSectionName() { return name; }

		mach_vm_address_t getAddress() { return address; }

		off_t getOffset() { return offset; }
		off_t getOffsetEnd() { return offset + size; }

		size_t getSize() { return size; }

		struct section_64* getSectionHeader() { return section; }

	private:
		struct section_64 *section;

		char *name;

		mach_vm_address_t address;

		off_t offset;

		size_t size;
};