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

namespace Debug
{
	using namespace Debug;

	class Dwarf;
	class CompilationUnit;
	class DIE;
	class LineTable;
	class AbbreviationTable;

	struct AttrAbbrev;

	Dwarf* parseDebugSymbols(MachO *macho, const char *dSYM);

	class DIE
	{
		public:
			explicit DIE(Dwarf *dwarf,
						 uint64_t code,
						 char *name,
						 enum DW_TAG tag,
						 enum DW_CHILDREN has_children);

			enum DW_TAG getTag() { return tag; }
			enum DW_CHILDREN getHasChildren() { return has_children; }

			uint64_t getCode() { return code; }

			uint32_t getAttributesCount() { return abbreviationTable.getSize(); }

			struct AttrAbbrev* getAttribute(enum DW_AT attr);
			struct AttrAbbrev* getAttribute(int index) { return abbreviationTable.get(index); }

			char* getName() { return name; }
			char* getMangledName();

			DIE* getParent() { return parent; }

			Array<DIE*>* getChildren() { return &children; }

			off_t getOffset() { return offset; }

			void setParent(DIE *die) { this->parent = die; }

			void setCompilationUnit(CompilationUnit *unit) { this->compilationUnit = unit; }

			void setOffset(off_t offset) { this->offset = offset; }

			void addChild(DIE *die);
			void removeChild(DIE *die);

			struct AttrAbbrev* getAbbreviation(int index) { return this->abbreviationTable.get(index); }
			struct AttrAbbrev* getAbbreviation(enum DW_AT attr);

			void addAbbreviation(struct AttrAbbrev *abbreviation) { this->abbreviationTable.add(abbreviation); }
			void removeAbbreviation(struct AttrAbbrev *abbreviation) { this->abbreviationTable.remove(abbreviation); }

		private:
			Dwarf *dwarf;

			uint64_t code;

			CompilationUnit *compilationUnit;

			Array<struct AttrAbbrev*> abbreviationTable;

			enum DW_TAG tag;

			enum DW_CHILDREN has_children;

			char *name;

			DIE *parent;

			Array<DIE*> children;

			uint32_t idx;
			uint32_t sibling_idx;
			uint32_t parent_idx;

			off_t offset;
	};

	#pragma pack(1)

	struct CompileUnitHeader
	{
		uint32_t length;
		uint16_t format;
		uint16_t version;
		uint16_t abbr_offset;
		uint8_t addr_size;
	};

	#pragma options align=reset

	class CompilationUnit
	{
		public:
			explicit CompilationUnit(Dwarf *dwarf, struct CompileUnitHeader *hdr, DIE *die);

			Dwarf* getDwarf() { return dwarf; }

			LineTable* getLineTable() { return lineTable; }

			char* getSourceFileName() { return source_file; }

		private:
			Dwarf *dwarf;

			DIE *die;

			struct CompileUnitHeader *header;

			LineTable *lineTable;
	
			char *source_file;
			char *source_language;

			uint16_t dwarf_version;

			uint8_t unit_type;
			uint64_t dwo_id;

			uint64_t unit_type_signature;
			uint64_t unit_type_offset;

			mach_vm_address_t address_base = 0;
			mach_vm_address_t str_offsets_base = 0;
			mach_vm_address_t range_lists_base = 0;
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

	struct AttrSpec
	{
		enum DW_AT name;
		enum DW_FORM form;
	};

	struct AttrAbbrev
	{
		enum DW_TAG tag;
		enum DW_CHILDREN children;

		struct AttrSpec attr_spec;

		uint64_t code;

		int64_t value;
	};

	class AbbreviationTable
	{
		public:
			explicit AbbreviationTable(CompilationUnit *cu, DIE *tag);

			void addAbbreviation(struct AttrAbbrev *abbreviation);
			void removeAbbreviation(struct AttrAbbrev *abbreviation);

		private:
			CompilationUnit *compilationUnit;

			DIE *tag;
	};

	class Dwarf
	{
		public:
			explicit Dwarf(const char *debugSymbols);
			explicit Dwarf(MachO *macho, const char *debugSymbols);

			CompilationUnit getCompilationUnit(const char *source_file);

			DIE* getDebugInfoEntryByName(const char *name);
			DIE* getDebugInfoEntryByCode(uint64_t code);

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
			Array<LineTable*>* getLineTables() { return &lineTables; }

			LineTable* getLineTable(const char *name);
			LineTable* getLineTable(CompilationUnit *unit);

			DIE* getUnit(const char *name);
			DIE* getFunction(const char *name);
			DIE* getType(const char *name);

			void populateDebugSymbols();

			void parseDebugAbbrev();
			void parseDebugInfo();
			void parseDebugLocations();

			const char* getSourceFile(mach_vm_address_t instruction);
			int64_t getSourceLineNumber(mach_vm_address_t instruction);

		private:
			MachO *macho;
			MachO *machoWithDebug;

			Array<DIE*> dies;
			Array<CompilationUnit*> compilationUnits;

			Array<LineTable*> lineTables;

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

	uint64_t GetStringSize(uint8_t *p);

	uint64_t ReadUleb128(uint8_t *p, uint8_t *end);
	uint64_t ReadUleb128(uint8_t *p, uint8_t *end, uint32_t *idx);

	int64_t ReadSleb128(uint8_t *p, uint8_t *end);
	int64_t ReadSleb128(uint8_t *p, uint8_t *end, uint32_t *idx);
};

#endif