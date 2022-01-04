#ifndef __SECTION_HPP_
#define __SECTION_HPP_

#include <mach/mach_types.h>
#include <sys/types.h>

#include <string.h>

#include "mach-o.h"

class Section
{
	public:
		Section(struct section_64 *section)
		{
			this->section = section;
			this->address = section->addr;
			this->offset = section->offset;
			this->size = section->size;

			this->name = new char[strlen(section->sectname) + 1];

			strlcpy(this->name, section->sectname, strlen(section->sectname));
		}

		~Section()
		{
			delete name;
		}

		struct section_64* getSection() { return section; }

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