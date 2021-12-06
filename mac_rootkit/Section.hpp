#ifndef __SECTION_HPP_
#define __SECTION_HPP_

#include <mach/mach_types.h>
#include <sys/types.h>

#include <string.h>

#include "Segment.hpp"

#include "mach-o.h"

class Section
{
	public:
		Section(struct section_64 *section)
		{
			this->section = sect;
			this->address = sect->addr;
			this->offset = sect->offset;
			this->size = sect->size;

			this->name = new char[strlen(sect->sectname)];

			strlcpy(this->name, sect->sectname, strlen(sect->sectname));
		}

		~Section()
		{
			delete name;
		}

		char* getSectionName() { return name; }

		mach_vm_address_t getAddress() { return address; }

		off_t getOffset() { return offset; }

		size_t getSize() { return size; }

		struct section_64* getSectionHeader() { return section; }

	private:
		struct section_64 *section;
		
		char *name;

		mach_vm_address_t address;

		off_t offset;

		size_t size;
};

#endif