#ifndef __MACHO_HPP_
#define __MACHO_HPP_

extern "C"
{
	#include <mach-o.h>
}

#include "BinaryFormat.hpp"

#include "vector.hpp"
#include "Symbol.hpp"
#include "SymbolTable.hpp"
#include "Segment.hpp"
#include "Section.hpp"

#include "Log.hpp"

#ifdef __USER__

#define max(x, y) (x > y ? x : y)
#define min(x, y) (x > y ? y : x)

#endif

class MachO : public Binary::BinaryFormat
{
	public:
		MachO();

		~MachO();

		virtual void initWithBase(mach_vm_address_t machoBase, off_t slide);

		struct mach_header_64* getMachHeader() { return header; }

		virtual mach_vm_address_t getBase() { return base; }

		mach_vm_address_t getEntryPoint() { return entry_point; }

		off_t getAslrSlide() { return aslr_slide; }

		virtual size_t getSize();

		uint8_t* getOffset(off_t offset) { return reinterpret_cast<uint8_t*>(buffer + offset); }
		uint8_t* getEnd() { return reinterpret_cast<uint8_t*>(buffer + getSize()); }

		std::vector<Segment*>& getSegments() { return segments; }
		
		std::vector<Section*>& getSections(Segment* segment);

		std::vector<Symbol*>& getAllSymbols() { return symbolTable->getAllSymbols(); }

		SymbolTable* getSymbolTable() { return symbolTable; }

		Symbol* getSymbol(char *symbolname) { return this->getSymbolByName(symbolname); }

		Symbol* getSymbolByName(char *symbolname);
		Symbol* getSymbolByAddress(mach_vm_address_t address);

		mach_vm_address_t getSymbolAddressByName(char *symbolname);

		off_t addressToOffset(mach_vm_address_t address);

		mach_vm_address_t offsetToAddress(off_t offset);

		void* addressToPointer(mach_vm_address_t address);

		mach_vm_address_t getBufferAddress(mach_vm_address_t address);

		Segment* getSegment(char *segmentname);
		Section* getSection(char *segmentname, char *sectionname);

		Segment* segmentForAddress(mach_vm_address_t address);
		Section* sectionForAddress(mach_vm_address_t address);

		Segment* segmentForOffset(off_t offset);
		Section* sectionForOffset(off_t offset);

		uint8_t* operator[](uint64_t index) { return this->getOffset(index); }

		virtual void parseSymbolTable(struct nlist_64 *symtab, uint32_t nsyms, char *strtab, size_t strsize);

		virtual void parseLinkedit();

		virtual bool parseLoadCommands();

		virtual void parseHeader();

		virtual void parseFatHeader();

		virtual void parseMachO();

	protected:
		char *buffer;

		bool fat;

		struct mach_header_64 *header;

		std::vector<Segment*> segments;

		SymbolTable *symbolTable;

		off_t aslr_slide;

		mach_vm_address_t entry_point;

		mach_vm_address_t base;

		size_t size;
};

#endif