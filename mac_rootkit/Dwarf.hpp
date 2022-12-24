#ifndef __DWARF_HPP_
#define __DWARF_HPP_

#include <DwarfV5.hpp>

class Symbol;
class SymbolTable;

class Segment;
class Section;

#include "Array.hpp"
#include "MachO.hpp"
#include "Symbol.hpp"
#include "SymbolTable.hpp"
#include "Segment.hpp"
#include "Section.hpp"

#include "Log.hpp"

namespace Dwarf
{
	using namespace Dwarf;

	class Dwarf;
	class CompilationUnit;
	class DIE;
	class LineTable;
	class AbbreviationTable;

	Dwarf* parseDebugSymbols(MachO *macho, const char *dSYM);

	class DIE
	{
		public:
			enum DW_TAG getTag() { return tag; }

			char* getName() { return name; }
			char* getMangledName();

			DIE* getParent() { return parent; }

			Array<DIE*>* getSiblings() { return &siblings; }
			Array<DIE*>* getChildren() { return &children; }

			off_t getOffset() { return offset; }

		private:
			Dwarf *dwarf;

			CompilationUnit *unit;

			enum DW_TAG tag;

			char *name;

			DIE *parent;

			Array<DIE*> siblings;
			Array<DIE*> children;

			uint32_t sibling_idx;
			uint32_t parent_idx;

			off_t offset;
	};

	class CompilationUnit
	{
		public:

			DIE* getDebugInfoEntry() { return die; }

			LineTable* getLineTable() { return lineTable; }

			char* getSourceFileName() { return source_file; }

		private:
			DIE *die;

			LineTable *lineTable;
	
			char *source_file;
	};

	class LineTable
	{
		struct SourceLine
		{
			mach_vm_address_t address;

			uint32_t line;
			uint16_t column;
			uint16_t file;
			uint32_t discriminator;

			uint8_t statement : 1,
					basic_block : 1,
					end_sequence : 1,
					prologue_end : 1,
					epilogue_begin : 1;
		};

		public:
			explicit LineTable(Dwarf *dwarf, CompilationUnit *unit);

			CompilationUnit* getCompilationUnit();

			SourceLine* getSourceLine(mach_vm_address_t pc);
			SourceLine* getSourceLine(uint32_t line, uint16_t column);

			void parseLineTable();

		private:
			CompilationUnit *unit;

			Array<SourceLine*> sources;
	};

	class AbbreviationTable
	{
		struct Abbreviation
		{
			enum DW_TAG tag;
			enum DW_CHILDREN children;

			struct AttrSpec
			{
				enum DW_AT name;
				enum DW_FORM form;
			} attr_spec;

			int64_t value;
		};

		public:

		private:
			Array<Abbreviation*> abbreviations;
	};

	class Dwarf
	{
		public:
			explicit Dwarf(const char *debugSymbols);
			explicit Dwarf(MachO *macho, const char *debugSymbols);

			CompilationUnit getCompilationUnit(const char *source_file);

			DIE* getDebugInfoEntry(const char *name);

			Segment* getDwarfSegment() { return dwarf; }

			Section* getDebugLine() { return __debug_line; }
			Section* getDebugLoc() { return __debug_loc; }
			Section* getDebugAranges() { return __debug_aranges; }
			Section* getDebugInfo() { return __debug_info; }
			Section* getDebugAbbrev() { return __debug_abbrev; }
			Section* getDebugStr() { return __debug_str; }

			Section* getAppleNames() { return __apple_names; }
			Section* getAppleNamespac() { return __apple_namespac; }
			Section* getAppleTypes() { return __apple_types; }
			Section* getAppleObjc() { return __apple_objc; }

			Array<CompilationUnit*>* getCompilationUnits() { return &compilationUnits; }
			Array<LineTable*>* getLineTables() { return &lineTable; }

			CompilationUnit* getCompilationUnit(const char *name);

			LineTable* getLineTable(const char *name);
			LineTable* getLineTable(CompilationUnit *unit);

			DIE* getUnit(const char *name);
			DIE* getFunction(const char *name);
			DIE* getType(const char *name);

			void parseDebugInfo();
			void parseDebugLocations();

			const char* getSourceFile(mach_vm_address_t instruction);
			int64_t getSourceLineNumber(mach_vm_address_t instruction);

		private:
			MachO *macho;
			MachO *machoWithDebug;

			Array<CompilationUnit*> compilationUnits;

			Array<LineTables*> *lineTables;

			AbbreviationTable *abbreviationTable;

			Segment *dwarf;

			Section *__debug_line;
			Section *__debug_loc;
			Section *__debug_aranges;
			Section *__debug_info;
			Section *__debug_ranges;
			Section *__debug_abbrev;
			Section *__debug_str;
			Section *__apple_names;
			Section *__apple_namespac;
			Section *__apple_types;
			Section *__apple_objc;

	};

	uint64_t ReadUleb128(uint8_t *p, uint8_t *end, uint32_t *idx);

	int64_t ReadSleb128(uint8_t *p, uint8_t *end, uint32_t *idx);
};

#endif