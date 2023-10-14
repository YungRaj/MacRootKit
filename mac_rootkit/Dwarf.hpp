#ifndef __DWARF_HPP_
#define __DWARF_HPP_

#include <DwarfV5.hpp>

#include "Log.hpp"

#include "vector.hpp"

#include "BinaryFormat.hpp"

namespace Binary
{
	class BinaryFormat;
};

namespace Debug
{
	using namespace Debug;

	template <typename BinaryFormat>
	struct BinaryFormatAttributes
	{
		using SegmentType = decltype(std::declval<BinaryFormat>()->getSegment(nullptr));
		using SectionType = decltype(std::declval<BinaryFormat>()->getSection(nullptr, nullptr));
		using SymbolType = decltype(std::declval<BinaryFormat>()->getSymbol(nullptr));
	};

	template <typename T>
	concept HasOperatorBracket = requires(T t, uint64_t index)
	{
	    { (*t) [index] } -> std::same_as<uint8_t*>;
	};

	template <typename T, typename Sym>
	concept HasCompatibleSymbol = std::is_same_v<Sym, decltype(std::declval<T>()->getSymbol(nullptr))>;

	template <typename T, typename Seg>
	concept HasCompatibleSegment = std::is_same_v<Seg, decltype(std::declval<T>()->getSegment(nullptr))>;

	template <typename T, typename Sect>
	concept HasCompatibleSect = std::is_same_v<Sect, decltype(std::declval<T>()->getSection(nullptr))>;

	template<typename T, typename Sym = typename BinaryFormatAttributes<T>::SymbolType, typename Seg = typename BinaryFormatAttributes<T>::SegmentType, typename Sect = typename BinaryFormatAttributes<T>::SectionType>
	concept DebuggableBinary = std::is_base_of_v<Binary::BinaryFormat, std::remove_pointer_t<T>> && HasOperatorBracket<T>;

	template<typename T> requires DebuggableBinary<T>
	class Dwarf;

	template<typename T> requires DebuggableBinary<T>
	class CompilationUnit;

	template<typename T> requires DebuggableBinary<T>
	class DIE;

	template<typename T> requires DebuggableBinary<T>
	class LineTable;

	template<typename T> requires DebuggableBinary<T>
	class AbbreviationTable;

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
	};

	struct Attribute
	{
		struct AttrAbbrev abbreviation;

		uint64_t value;
	};

	template<typename T, typename Sym = typename BinaryFormatAttributes<T>::SymbolType, typename Seg = typename BinaryFormatAttributes<T>::SegmentType, typename Sect = typename BinaryFormatAttributes<T>::SectionType> requires DebuggableBinary<T, Sym, Seg>
	Dwarf<T>* parseDebugSymbols(T binary);

	template<typename T, typename Sym = typename BinaryFormatAttributes<T>::SymbolType, typename Seg = typename BinaryFormatAttributes<T>::SegmentType, typename Sect = typename BinaryFormatAttributes<T>::SectionType> requires DebuggableBinary<T, Sym, Seg>
	Dwarf<T>* parseDebugSymbols(T binary, const char *dSYM);

	template<typename T> requires DebuggableBinary<T>
	class DIE
	{
		public:
			explicit DIE(Dwarf<T> *dwarf,
						 uint64_t code,
						 char *name,
						 enum DW_TAG tag,
						 enum DW_CHILDREN has_children);

			enum DW_TAG getTag() { return tag; }
			enum DW_CHILDREN getHasChildren() { return has_children; }

			uint64_t getCode() { return code; }

			size_t getAttributesCount() { return abbreviationTable.size(); }

			struct AttrAbbrev* getAttribute(enum DW_AT attr);
			struct AttrAbbrev* getAttribute(int index) { return abbreviationTable.at(index); }

			char* getName() { return name; }

			off_t getOffset() { return offset; }

			void setOffset(off_t offset) { this->offset = offset; }

			struct AttrAbbrev* getAbbreviation(int index) { return this->abbreviationTable.at(index); }
			struct AttrAbbrev* getAbbreviation(enum DW_AT attr);

			void addAbbreviation(struct AttrAbbrev *abbreviation) { this->abbreviationTable.push_back(abbreviation); }
			void removeAbbreviation(struct AttrAbbrev *abbreviation) { this->abbreviationTable.erase(std::remove(abbreviationTable.begin(), abbreviationTable.end(), abbreviation), abbreviationTable.end()); }

		private:
			Dwarf<T> *dwarf;

			uint64_t code;

			std::vector<struct AttrAbbrev*> abbreviationTable;

			enum DW_TAG tag;

			enum DW_CHILDREN has_children;

			char *name;

			DIE<T> *parent;

			uint32_t idx;
			uint32_t sibling_idx;
			uint32_t parent_idx;

			off_t offset;
	};

	template<typename T> requires DebuggableBinary<T>
	class DwarfDIE
	{
		public:
			explicit DwarfDIE(Dwarf<T> *dwarf,
							  CompilationUnit<T> *unit,
							  DIE<T> *die,
							  DwarfDIE<T> *parent);

			Dwarf<T>* getDwarf() { return dwarf; }

			DIE<T>* getDebugInfoEntry() { return die; }

			enum DW_TAG getTag() { return die->getTag(); }
			enum DW_CHILDREN hasChildren() { return die->getHasChildren(); }

			DwarfDIE<T>* getParent() { return parent; }

			std::vector<DwarfDIE<T>*>& getChildren() { return children; }

			std::vector<struct Attribute*>& getAttributes() { return attributes; }

			void addChild(DwarfDIE<T> *child) { this->children.push_back(child); }
			void removeChild(DwarfDIE<T> *child) { this->children.erase(std::remove(children.begin(), children.end(), child), children.end()); }

			void addAttribute(struct Attribute *attribute) { this->attributes.push_back(attribute); }
			void addAttributes(std::vector<struct Attribute*> &attrs) { for(int i = 0; i < attrs.size(); i++) this->attributes.push_back(attrs.at(i)); }

			struct Attribute* getAttribute(enum DW_AT attr);
			struct Attribute* getAttribute(int index) { return this->attributes.at(index); }

			uint64_t getAttributeValue(enum DW_AT attr);

		private:
			Dwarf<T> *dwarf;

			CompilationUnit<T> *compilationUnit;

			DIE<T> *die;

			DwarfDIE<T> *parent;

			std::vector<DwarfDIE*> children;
			std::vector<struct Attribute*> attributes;
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

	template<typename T> requires DebuggableBinary<T>
	class CompilationUnit
	{
		public:
			explicit CompilationUnit(Dwarf<T> *dwarf, struct CompileUnitHeader *hdr, DIE<T> *die);

			std::vector<DwarfDIE<T>*>& getDebugInfoEntries() { return debugInfoEntries; }

			Dwarf<T>* getDwarf() { return dwarf; }

			LineTable<T>* getLineTable() { return lineTable; }

			char* getSourceFileName() { return source_file; }

			void addDebugInfoEntry(DwarfDIE<T> *dwarfDIE) { this->debugInfoEntries.push_back(dwarfDIE); }

		private:
			Dwarf<T> *dwarf;

			DIE<T> *die;

			struct CompileUnitHeader *header;

			std::vector<DwarfDIE<T>*> debugInfoEntries;

			LineTable<T> *lineTable;
	
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

	#pragma pack(1)

	struct LTPrologue
	{
		uint32_t total_length;
		uint8_t format;
		uint8_t version;
		uint32_t prologue_length;
		uint8_t min_inst_length;
		uint8_t max_ops_per_inst;
		uint8_t default_is_stmt;
		int8_t line_base;
		uint8_t line_range;
		uint8_t opcode_base;
	};

	struct LTStandardOpcodeLengths
	{
		DW_LNS copy;
		DW_LNS advance_pc;
		DW_LNS advance_line;
		DW_LNS set_file;
		DW_LNS set_column;
		DW_LNS negate_stmt;
		DW_LNS set_basic_block;
		DW_LNS const_add_pc;
		DW_LNS fixed_advance_pc;
		DW_LNS set_prologue_end;
		DW_LNS set_epilogue_begin;
		DW_LNS set_isa;
	};

	struct LTSourceFileMetadata
	{
		uint8_t dir_index;
		uint8_t mod_time;
		uint8_t length;
	};

	#pragma options align=reset

	struct LTSourceFile
	{
		char *source_file;

		struct LTSourceFileMetadata metadata;
	};

	struct LTStateMachine
	{
		mach_vm_address_t address;

		uint8_t  isa;
		int64_t line;
		uint64_t column;
		uint16_t file;
		uint32_t discriminator;

		uint8_t statement : 1,
				basic_block : 1,
				end_sequence : 1,
				prologue_end : 1,
				epilogue_begin : 1;
	};

	struct LTSourceLine
	{
		struct LTSourceFile *source_file;

		struct LTStateMachine state;
	};

	struct Sequence
	{
		uint64_t LowPC;
		uint64_t HighPC;

		Segment *segment;
		Section *section;

		std::vector<LTSourceLine*> sourceLines;
	};

	static LTStateMachine gInitialState =
	{
		.address = 0,
		.isa = 0,
		.line = 1,
		.column = 1,
		.file = 1,
		.discriminator = 1,
		.statement = 1,
		.basic_block = 0,
		.end_sequence = 0,
		.prologue_end = 0,
		.epilogue_begin = 0
	};

	template<typename T> requires DebuggableBinary<T>
	class LineTable
	{
		public:
			explicit LineTable(T binary, Dwarf<T> *dwarf) { this->binary = binary; this->dwarf = dwarf; }

			std::vector<struct LTSourceFile*>& getSourceFileNames() { return files; }
			std::vector<char*>& getIncludeDirectories() { return include_directories; }

			CompilationUnit<T>* getCompilationUnit() { return compilationUnit; }

			LTSourceLine* getSourceLine(mach_vm_address_t pc);
			
			LTSourceFile* getSourceFile(int index) { return this->files.at(index); }

			void setCompilationUnit(CompilationUnit<T> *cu) { this->compilationUnit = cu; }

			void setPrologue(struct LTPrologue *p) { memcpy(&prologue, p, sizeof(struct LTPrologue)); }

			void setStandardOpcodeLengths(struct LTStandardOpcodeLengths *opcodes) { memcpy(&standardOpcodeLengths, opcodes, sizeof(struct LTStandardOpcodeLengths)); }

			void addSequence(Sequence *sequence) { this->sources.push_back(sequence); }
			void addSourceFile(struct LTSourceFile *file) { this->files.push_back(file); }
			void addIncludeDirectory(char *directory) { this->include_directories.push_back(directory); }

		private:
			T binary;

			Dwarf<T> *dwarf;

			struct LTPrologue prologue;

			struct LTStandardOpcodeLengths standardOpcodeLengths;

			CompilationUnit<T> *compilationUnit;

			std::vector<Sequence*> sources;

			std::vector<char*> include_directories;
			std::vector<struct LTSourceFile*> files;
	};

	struct LocationTableEntry
	{
		DW_LLE kind;

		uint32_t offset;

		uint64_t value0;
		uint64_t value1;

		Segment *segment;

		std::vector<DW_OP> location_ops;
	};

	#pragma pack(1)

	struct AddressRange
	{
		mach_vm_address_t start;
		mach_vm_address_t end;
	};

	struct AddressRangeHeader
	{
		uint32_t length;
		uint16_t version;
		uint32_t offset;
		uint8_t addr_size;
		uint8_t seg_size;
	};

	struct AddressRangeEntry
	{
		struct AddressRangeHeader header;

		std::vector<struct AddressRange*> ranges;
	};

	struct RangeEntry
	{
		uint32_t offset;

		uint64_t value0;
		uint64_t value1;
	};

	using RangeEntries = std::vector<struct RangeEntry*>;

	#pragma options align=reset

	template<typename T> requires DebuggableBinary<T>
	class Dwarf
	{
		public:
			using Seg = typename BinaryFormatAttributes<T>::SegmentType;
			using Sect = typename BinaryFormatAttributes<T>::SectionType;
			using Sym = typename BinaryFormatAttributes<T>::SymbolType;

			explicit Dwarf(const char *debugSymbols);
			explicit Dwarf(T binary, const char *debugSymbols);

			CompilationUnit<T> getCompilationUnit(const char *source_file);

			DIE<T>* getDebugInfoEntryByName(const char *name);
			DIE<T>* getDebugInfoEntryByCode(uint64_t code);

			Seg getDwarfSegment() { return dwarf; }

			Sect getDebugLine() { return __debug_line; }
			Sect getDebugLoc() { return __debug_loc; }
			Sect getDebugAranges() { return __debug_aranges; }
			Sect getDebugInfo() { return __debug_info; }
			Sect getDebugAbbrev() { return __debug_abbrev; }
			Sect getDebugStr() { return __debug_str; }

			Sect getAppleNames() { return __apple_names; }
			Sect getAppleNamespac() { return __apple_namespac; }
			Sect getAppleTypes() { return __apple_types; }
			Sect getAppleObjc() { return __apple_objc; }

			std::vector<CompilationUnit<T>*>& getCompilationUnits() { return compilationUnits; }
			std::vector<LineTable<T>*>& getLineTables() { return lineTables; }

			LineTable<T>* getLineTable(const char *name);
			LineTable<T>* getLineTable(CompilationUnit<T> *unit);

			DIE<T>* getUnit(const char *name);
			DIE<T>* getFunction(const char *name);
			DIE<T>* getType(const char *name);

			void populateDebugSymbols();

			void parseDebugAbbrev();
			void parseDebugInfo();
			void parseDebugLocations();
			void parseDebugLines();
			void parseDebugRanges();
			void parseDebugAddressRanges();

			const char* getSourceFile(mach_vm_address_t instruction);

			int64_t getSourceLineNumber(mach_vm_address_t instruction);

		private:
			T binary;

			T binaryWithDebugSymbols;

			std::vector<DIE<T>*> dies;
			std::vector<CompilationUnit<T>*> compilationUnits;

			std::vector<LineTable<T>*> lineTables;

			std::vector<struct LocationTableEntry*> locationTable;

			std::vector<RangeEntries*> ranges;
			std::vector<struct AddressRangeEntry*> addressRanges;

			Seg dwarf;

			Sect __debug_line;
			Sect __debug_loc;
			Sect __debug_aranges;
			Sect __debug_info;
			Sect __debug_ranges;
			Sect __debug_abbrev;
			Sect __debug_str;
			Sect __apple_names;
			Sect __apple_namespac;
			Sect __apple_types;
			Sect __apple_objc;

	};

	uint64_t GetStringSize(uint8_t *p);

	uint64_t ReadUleb128(uint8_t *p, uint8_t *end);
	uint64_t ReadUleb128(uint8_t *p, uint8_t *end, uint32_t *idx);

	int64_t ReadSleb128(uint8_t *p, uint8_t *end);
	int64_t ReadSleb128(uint8_t *p, uint8_t *end, uint32_t *idx);
};

#endif