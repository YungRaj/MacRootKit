#ifdef __MACHO_HPP_
#define __MACHO_HPP_

#include "mach-o.h"

#include "Array.hpp"
#include "Symbol.hpp"
#include "SymbolTable.hpp"
#include "Segment.hpp"
#include "Section.hpp"

#include "Log.hpp"

class Symbol;
class SymbolTable;

class Segment;
class Section;

class MachO
{
	public:
		MachO() {}

		~MachO();

		virtual void initWithBase(mach_vm_address_t base, off_t slide);

		struct mach_header_64* getMachHeader() { return header; }

		mach_vm_address_t getBase() { return base; }

		mach_vm_address_t getEntryPoint() { return entry_point; }

		off_t getAslrSlide() { return aslr_slide; }

		size_t getSize();

		uint8_t* getOffset(off_t offset) { return static_cast<uint8_t*>(buffer + offset); }

		Array<Segment*>* getSegments() { return &segments; }
		
		Array<Section*>* getSections(Segment* segment);

		SymbolTable* getSymbolTable() { return symbol_table; }

		Symbol* getSymbolByName(char *symbolname);
		Symbol* getSymbolByAddress(mach_vm_address_t address);

		mach_vm_address_t getSymbolAddressByName(char *symbolname);

		off_t addressToOffset(mach_vm_address_t address);

		mach_vm_address_t offsetToAddress(off_t offset);

		Segment* segmentForAddress(mach_vm_address_t address);
		Section* sectionForAddress(mach_vm_address_t address);

		Segment* segmentForOffset(off_t offset);
		Section* sectionForOffset(off_t offset);

		virtual void parseSymbolTable(struct nlist_64 *symtab, uint32_t nsyms, char *strtab, size_t strsize);

		virtual void parseLinkedit();

		virtual void parseLoadCommands();

		virtual void parseFatHeader();

		virtual void parseMachO();

	protected:
		char *buffer;

		bool fat;

		struct mach_header_64 *header;

		Array<Segment> *segments;

		SymbolTable *symbol_table;

		off_t aslr_slide;

		mach_vm_address_t entry_point;

		mach_vm_address_t base;

		size_t size;
}