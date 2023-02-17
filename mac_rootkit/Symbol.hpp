#ifndef __SYMBOL_HPP_
#define __SYMBOL_HPP_

#include <cxxabi.h>

#include "MachO.hpp"

#include "Segment.hpp"
#include "Section.hpp"

class MachO;

class Segment;
class Section;

#ifdef 0

char* cxx_demangle(char *name)
{
	int status;

	char *ret = abi::__cxa_demangle(abiName, 0, 0, &status);  

	std::shared_ptr<char> retval;

	retval.reset( (char *)ret, [](char *mem) { if (mem) free((void*)mem); } );

	return retval;
}

typedef char* (*_swift_demangle) (uint32_t length, uint8_t *output_buffer, uint32_t output_buffer_size, uint32_t flags);

char* swift_demangle(char *name)
{
	void *RTLD_DEFAULT = dlopen(NULL, RTLD_NOW);

	mach_vm_address_t sym = dlsym(RTLD_DEFAULT, "swift_demangle");

	if(sym)
	{
		_swift_demangle f = reinterpret_cast<_swift_demangle>(sym);

		char *cString = f(mangled, strlen(mangled), NULL, NULL, 0);
		
		if(cString)
		{
			return cString;
		}
	}

	return NULL;
}

#endif

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

		char *getDemangledName() { return NULL; }

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