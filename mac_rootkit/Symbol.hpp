#ifndef __SYMBOL_HPP_
#define __SYMBOL_HPP_

#include "MachO.hpp"

#include "Segment.hpp"
#include "Section.hpp"

class MachO;

class Segment;
class Section;

extern "C"
{
	extern char* cxx_demangle(char *mangled);
	extern char* swift_demangle(char *mangled);
}

class Symbol
{
	public:
		Symbol() { }

		Symbol(MachO *macho, uint32_t type, char *name, mach_vm_address_t address, off_t offset, Segment *segment, Section *section)
		{
			this->macho = macho;
			this->type = type;
			this->name = name;
			this->demangled_name = NULL; // this->getDemangledName();
			this->address = address;
			this->offset = offset;
			this->segment = segment;
			this->section = section;
		}

		bool is_cxx() { return cxx_demangle(this->getName()) != NULL; }
		bool is_swift() { return cxx_demangle(this->getName()) != NULL; }

		MachO* getMachO() { return macho; }

		Segment* getSegment() { return segment; }

		Section* getSection() { return section; }

		char* getName() { return name; }

		char* getDemangledName()
		{
			char *_swift_demangle = swift_demangle(this->getName());
			char *_cxx_demangle = cxx_demangle(this->getName());

			return _swift_demangle ? _swift_demangle : (_cxx_demangle ? _cxx_demangle : strdup(""));
		}

		mach_vm_address_t getAddress() { return address; }

		off_t getOffset() { return offset; }

		uint32_t getType() { return type; }

	private:
		MachO *macho;

		Segment *segment;
		Section *section;

		char *name;
		char *demangled_name;

		uint32_t type;

		mach_vm_address_t address;

		off_t offset;
};

#endif