#ifndef __SYMBOL_HPP_
#define __SYMBOL_HPP_

#include "MachO.hpp"

#include "Segment.hpp"
#include "Section.hpp"

class MachO;

class Segment;
class Section;

class Symbol
{
	public:
		Symbol() { }

		Symbol(MachO *macho, uint32_t type, char *name, mach_vm_address_t address, off_t offset, Segment *segment, Section *section)
		{
			this->macho = macho;
			this->type = type;
			this->name = name;
			this->address = address;
			this->offset = offset;
			this->segment = segment;
			this->section = section;
		}

		MachO* getMachO() { return macho; }

		Segment* getSegment() { return segment; }

		Section* getSection() { return section; }

		char* getName() { return name; }

		mach_vm_address_t getAddress() { return address; }

		off_t getOffset() { return offset; }

		uint32_t getType() { return type; }

	private:
		MachO *macho;

		Segment *segment;
		Section *section;

		char *name;

		uint32_t type;

		mach_vm_address_t address;

		off_t offset;
};

#endif