#include "Dwarf.hpp"
#include "KernelMachO.hpp"

#include <string.h>

using namespace Debug;

char* DWTagToString(enum DW_TAG tag)
{
	switch(tag)
	{
		case DW_TAG::array_type:
			return "array_type";
		case DW_TAG::class_type:
			return "class_type";
		case DW_TAG::entry_point:
			return "entry_point";
		case DW_TAG::enumeration_type:
			return "enumeration_type";
		case DW_TAG::formal_parameter:
			return "formal_parameter";
		case DW_TAG::imported_declaration:
			return "imported_declaration";
		case DW_TAG::label:
			return "label";
		case DW_TAG::lexical_block:
			return "lexical_block";
		case DW_TAG::member:
			return "member";
		case DW_TAG::pointer_type:
			return "pointer_type";
		case DW_TAG::reference_type:
			return "reference_type";
		case DW_TAG::compile_unit:
			return "compile_unit";
		case DW_TAG::string_type:
			return "string_type";
		case DW_TAG::structure_type:
			return "structure_type";
		case DW_TAG::subroutine_type:
			return "subroutine_type";
		case DW_TAG::typedef_:
			return "typedef_";
		case DW_TAG::union_type:
			return "union_type";
		case DW_TAG::unspecified_parameters:
			return "unspecified_parameters";
		case DW_TAG::variant:
			return "variant";
		case DW_TAG::common_block:
			return "common_block";
		case DW_TAG::common_inclusion:
			return "common_inclusion";
		case DW_TAG::inheritance:
			return "inheritance";
		case DW_TAG::inlined_subroutine:
			return "inlined_subroutine";
		case DW_TAG::module:
			return "module";
		case DW_TAG::ptr_to_member_type:
			return "ptr_to_member_type";
		case DW_TAG::set_type:
			return "set_type";
		case DW_TAG::subrange_type:
			return "subrange_type";
		case DW_TAG::with_stmt:
			return "with_stmt";
		case DW_TAG::access_declaration:
			return "access_declaration";
		case DW_TAG::base_type:
			return "base_type";
		case DW_TAG::catch_block:
			return "catch_block";
		case DW_TAG::const_type:
			return "const_type";
		case DW_TAG::constant:
			return "constant";
		case DW_TAG::enumerator:
			return "enumerator";
		case DW_TAG::file_type:
			return "file_type";
		case DW_TAG::friend_:
			return "friend_";
		case DW_TAG::namelist:
			return "namelist";
		case DW_TAG::namelist_item:
			return "namelist_item";
		case DW_TAG::packed_type:
			return "packed_type";
		case DW_TAG::subprogram:
			return "subprogram";
		case DW_TAG::template_type_parameter:
			return "template_type_parameter";
		case DW_TAG::template_value_parameter:
			return "template_value_parameter";
		case DW_TAG::thrown_type:
			return "thrown_type";
		case DW_TAG::try_block:
			return "try_block";
		case DW_TAG::variant_part:
			return "variant_part";
		case DW_TAG::variable:
			return "variable";
		case DW_TAG::volatile_type:
			return "volatile_type";
		case DW_TAG::dwarf_procedure:
			return "dwarf_procedure";
		case DW_TAG::restrict_type:
			return "restrict_type";
		case DW_TAG::interface_type:
			return "interface_type";
		case DW_TAG::namespace_:
			return "namespace_";
		case DW_TAG::imported_module:
			return "imported_module";
		case DW_TAG::unspecified_type:
			return "unspecified_type";
		case DW_TAG::partial_unit:
			return "partial_unit";
		case DW_TAG::imported_unit:
			return "imported_unit";
		case DW_TAG::condition:
			return "condition";
		case DW_TAG::shared_type:
			return "shared_type";
		case DW_TAG::type_unit:
			return "type_unit";
		case DW_TAG::rvalue_reference_type:
			return "rvalue_reference_type";
		case DW_TAG::template_alias:
			return "template_alias";
		case DW_TAG::lo_user:
			return "lo_user";
		case DW_TAG::hi_user:
			return "hi_user";
	}

	char *ret = new char[1024];
	
	snprintf(ret, 1024, "unknown 0x%llx", static_cast<uint32_t>(tag));

	return ret;
}


char* DWAttrToString(enum DW_AT attr)
{
	switch(attr)
	{
		case DW_AT::sibling:
			return "sibling";
		case DW_AT::location:
			return "location";
		case DW_AT::name:
			return "name";
		case DW_AT::ordering:
			return "ordering";
		case DW_AT::byte_size:
			return "byte_size";
		case DW_AT::bit_offset:
			return "bit_offset";
		case DW_AT::bit_size:
			return "bit_size";
		case DW_AT::stmt_list:
			return "stmt_list";
		case DW_AT::low_pc:
			return "low_pc";
		case DW_AT::high_pc:
			return "high_pc";
		case DW_AT::language:
			return "language";
		case DW_AT::discr:
			return "discr";
		case DW_AT::discr_value:
			return "discr_value";
		case DW_AT::visibility:
			return "visibility";
		case DW_AT::import:
			return "import";
		case DW_AT::string_length:
			return "string_length";
		case DW_AT::common_reference:
			return "common_reference";
		case DW_AT::comp_dir:
			return "comp_dir";
		case DW_AT::const_value:
			return "const_value";
		case DW_AT::containing_type:
			return "containing_type";
		case DW_AT::default_value:
			return "default_value";
		case DW_AT::inline_:
			return "inline_";
		case DW_AT::is_optional:
			return "is_optional";
		case DW_AT::lower_bound:
			return "lower_bound";
		case DW_AT::producer:
			return "producer";
		case DW_AT::prototyped:
			return "prototyped";
		case DW_AT::return_addr:
			return "return_addr";
		case DW_AT::start_scope:
			return "start_scope";
		case DW_AT::bit_stride:
			return "bit_stride";
		case DW_AT::upper_bound:
			return "upper_bound";
		case DW_AT::abstract_origin:
			return "abstract_origin";
		case DW_AT::accessibility:
			return "accessibility";
		case DW_AT::address_class:
			return "address_class";
		case DW_AT::artificial:
			return "artificial";
		case DW_AT::base_types:
			return "base_types";
		case DW_AT::calling_convention:
			return "calling_convention";
		case DW_AT::count:
			return "count";
		case DW_AT::data_member_location:
			return "data_member_location";
		case DW_AT::decl_column:
			return "decl_column";
		case DW_AT::decl_line:
			return "decl_line";
		case DW_AT::decl_file:
			return "decl_file";
		case DW_AT::declaration:
			return "declaration";
		case DW_AT::discr_list:
			return "discr_list";
		case DW_AT::encoding:
			return "encoding";
		case DW_AT::frame_base:
			return "frame_base";
		case DW_AT::friend_:
			return "friend_";
		case DW_AT::identifier_case:
			return "identifier_case";
		case DW_AT::macro_info:
			return "macro_info";
		case DW_AT::namelist_item:
			return "namelist_item";
		case DW_AT::priority:
			return "priority";
		case DW_AT::segment:
			return "segment";
		case DW_AT::specification:
			return "specification";
		case DW_AT::static_link:
			return "static_link";
		case DW_AT::type:
			return "type";
		case DW_AT::use_location:
			return "use_location";
		case DW_AT::variable_parameter:
			return "variable_parameter";
		case DW_AT::virtuality:
			return "virtuality";
		case DW_AT::vtable_elem_location:
			return "vtable_elem_location";
		case DW_AT::allocated:
			return "allocated";
		case DW_AT::associated:
			return "associated";
		case DW_AT::data_location:
			return "data_location";
		case DW_AT::byte_stride:
			return "byte_stride";
		case DW_AT::entry_pc:
			return "entry_pc";
		case DW_AT::use_UTF8:
			return "use_UTF8";
		case DW_AT::extension:
			return "extension";
		case DW_AT::ranges:
			return "ranges";
		case DW_AT::trampoline:
			return "trampoline";
		case DW_AT::call_column:
			return "call_column";
		case DW_AT::call_file:
			return "call_file";
		case DW_AT::description:
			return "description";
		case DW_AT::binary_scale:
			return "binary_scale";
		case DW_AT::decimal_scale:
			return "decimal_scale";
		case DW_AT::small:
			return "small";
		case DW_AT::decimal_sign:
			return "decimal_sign";
		case DW_AT::digit_count:
			return "digit_count";
		case DW_AT::picture_string:
			return "picture_string";
		case DW_AT::mutable_:
			return "mutable_";
		case DW_AT::threads_scaled:
			return "threads_scaled";
		case DW_AT::explicit_:
			return "explicit_";
		case DW_AT::object_pointer:
			return "object_pointer";
		case DW_AT::endianity:
			return "endianity";
		case DW_AT::elemental:
			return "elemental";
		case DW_AT::pure:
			return "pure";
		case DW_AT::recursive:
			return "recursive";
		case DW_AT::signature:
			return "signature";
		case DW_AT::main_subprogram:
			return "main_subprogram";
		case DW_AT::data_bit_offset:
			return "data_bit_offset";
		case DW_AT::const_expr:
			return "const_expr";
		case DW_AT::enum_class:
			return "enum_class";
		case DW_AT::linkage_name:
			return "linkage_name";
		case DW_AT::lo_user:
			return "lo_user";
		case DW_AT::hi_user:
			return "hi_user";
		case DW_AT::external:
			return "external";
		case DW_AT::call_line:
			return "call_line";
	}

	char *ret = new char[1024];
	
	snprintf(ret, 1024, "unknown 0x%llx", static_cast<uint32_t>(attr));

	return ret;
}

char* DWFormToString(enum DW_FORM form)
{
	switch(form)
	{
		case DW_FORM::addr:
			return "addr";
		case DW_FORM::block2:
			return "block2";
		case DW_FORM::block4:
			return "block4";
		case DW_FORM::data1:
			return "data1";
		case DW_FORM::data2:
			return "data2";
		case DW_FORM::data4:
			return "data4";
		case DW_FORM::data8:
			return "data8";
		case DW_FORM::string:
			return "string";
		case DW_FORM::block:
			return "block";
		case DW_FORM::block1:
			return "block1";
		case DW_FORM::ref_addr:
			return "ref_addr";
		case DW_FORM::udata:
			return "udata";
		case DW_FORM::flag:
			return "flag";
		case DW_FORM::strp:
			return "strp";
		case DW_FORM::ref1:
			return "ref1";
		case DW_FORM::ref2:
			return "ref2";
		case DW_FORM::ref4:
			return "ref4";
		case DW_FORM::ref8:
			return "ref8";
		case DW_FORM::ref_udata:
			return "ref_udata";
		case DW_FORM::indirect:
			return "indirect";
		case DW_FORM::sec_offset:
			return "sec_offset";
		case DW_FORM::exprloc:
			return "exprloc";
		case DW_FORM::flag_present:
			return "flag_present";
		case DW_FORM::ref_sig8:
			return "ref_sig8";
		case DW_FORM::sdata:
			return "ref_sdata";
	}

	char *ret = new char[1024];
	
	snprintf(ret, 1024, "unknown 0x%llx", static_cast<uint32_t>(form));

	return ret;
}

size_t DWFormSize(enum DW_FORM form)
{
	switch(form)
	{
		case DW_FORM::addr:
			return 8;
		case DW_FORM::block2:
			return 2;
		case DW_FORM::block4:
			return 4;
		case DW_FORM::data1:
			return 1;
		case DW_FORM::data2:
			return 2;
		case DW_FORM::data4:
			return 4;
		case DW_FORM::data8:
			return 8;
		case DW_FORM::string:
			return 4;
		case DW_FORM::block:
			return 8;
		case DW_FORM::block1:
			return 8;
		case DW_FORM::ref_addr:
			return 4;
		case DW_FORM::udata:
			return 4;
		case DW_FORM::flag:
			return 4;
		case DW_FORM::strp:
			return 4;
		case DW_FORM::ref1:
			return 1;
		case DW_FORM::ref2:
			return 2;
		case DW_FORM::ref4:
			return 4;
		case DW_FORM::ref8:
			return 8;
		case DW_FORM::ref_udata:
			return 4;
		case DW_FORM::indirect:
			return 4;
		case DW_FORM::sec_offset:
			return 4;
		case DW_FORM::exprloc:
			return 4;
		case DW_FORM::flag_present:
			return 0;
		case DW_FORM::ref_sig8:
			return 8;
	}

	return 0;
}

char* SourceLineFlagsToString(struct LTSourceLine *sourceLine)
{
	char *buffer = new char[1024];

	snprintf(buffer, 1024, "%s %s %s %s %s", sourceLine->state.statement > 0 ? "is_stmt" : "", sourceLine->state.basic_block > 0 ? "basic_block" : "", sourceLine->state.end_sequence > 0 ? "end_sequence" : "", sourceLine->state.prologue_end > 0 ? "prologue_end" : "", sourceLine->state.epilogue_begin > 0 ? "epilogue_begin" : "");

	return buffer;
}

uint64_t Debug::GetStringSize(uint8_t *p)
{
	uint8_t *s = p;

	uint64_t size = 0;

	while(*s++) { size++; }

	return size;
}

uint64_t Debug::ReadUleb128(uint8_t *p, uint8_t *end)
{
	uint64_t result = 0;

	int bit = 0;

	do
	{
		if(p == end)
		{
			MAC_RK_LOG("malformed uleb128\n");

			break;
		}

		uint64_t slice = *p & 0x7F;

		if(bit > 63)
		{
			MAC_RK_LOG("uleb128 too big for uint64\n");

			break;
		} else 
		{
			result |= (slice << bit);

			bit += 7;
		}

	} while(*p++ & 0x80);

	return result;
}

uint64_t Debug::ReadUleb128(uint8_t *p, uint8_t *end, uint32_t *idx)
{
	uint64_t result = 0;

	int bit = 0;

	do
	{
		if(p == end)
		{
			MAC_RK_LOG("malformed uleb128\n");

			break;
		}

		uint64_t slice = *p & 0x7F;

		if(bit > 63)
		{
			MAC_RK_LOG("uleb128 too big for uint64\n");

			break;
		} else 
		{
			result |= (slice << bit);

			bit += 7;
		}

		*idx = *idx + 1;

	} while(*p++ & 0x80);

	return result;
}

int64_t Debug::ReadSleb128(uint8_t *p, uint8_t *end)
{
	int64_t result = 0;

	int bit = 0;

	uint8_t byte = 0;

	do
	{
		if(p == end)
		{
			MAC_RK_LOG("malformed sleb128\n");

			break;
		}

		byte = *p++;

		result |= (((int64_t) (byte & 0x7f)) << bit);

		bit += 7;
	} while (byte & 0x80);
	// sign extend negative numbers

	if(((byte & 0x40) != 0) && (bit < 64))
		result |= (~0ULL) << bit;

	return result;
}

int64_t Debug::ReadSleb128(uint8_t *p, uint8_t *end, uint32_t *idx)
{
	int64_t result = 0;

	int bit = 0;

	uint8_t byte = 0;

	do
	{
		if(p == end)
		{
			MAC_RK_LOG("malformed sleb128\n");

			break;
		}

		byte = *p++;

		*idx = *idx + 1;

		result |= (((int64_t) (byte & 0x7f)) << bit);

		bit += 7;
	} while (byte & 0x80);
	// sign extend negative numbers

	if(((byte & 0x40) != 0) && (bit < 64))
		result |= (~0ULL) << bit;

	return result;
}

using namespace Debug;

DIE::DIE(Dwarf *dwarf,
		 uint64_t code,
		 char *name,
		 enum DW_TAG tag,
		 enum DW_CHILDREN has_children)
{
	this->dwarf = dwarf;
	this->code = code;
	this->name = name;
	this->tag = tag;
	this->has_children = has_children;
}

struct AttrAbbrev* DIE::getAttribute(enum DW_AT attr)
{
	for(int i = 0; i < abbreviationTable.getSize(); i++)
	{
		struct AttrAbbrev *ab = abbreviationTable.get(i);

		if(attr == ab->attr_spec.name)
		{
			return ab;
		}
	}

	return NULL;
}

DwarfDIE::DwarfDIE(Dwarf *dwarf,
				   CompilationUnit *unit,
				   DIE *die,
				   DwarfDIE *parent)
{
	this->dwarf = dwarf;
	this->compilationUnit = unit;
	this->die = die;
	this->parent = parent;
}

struct Attribute* DwarfDIE::getAttribute(enum DW_AT attr)
{
	for(int i = 0; i < this->attributes.getSize(); i++)
	{
		struct Attribute *attribute = this->attributes.get(i);

		if(attribute->abbreviation.attr_spec.name == attr)
			return attribute;
	}

	return NULL;
}

uint64_t DwarfDIE::getAttributeValue(enum DW_AT attr)
{
	for(int i = 0; i < this->attributes.getSize(); i++)
	{
		struct Attribute *attribute = this->attributes.get(i);

		if(attribute->abbreviation.attr_spec.name == attr)
			return attribute->value;
	}

	return 0;
}

CompilationUnit::CompilationUnit(Dwarf *dwarf, struct CompileUnitHeader *hdr, DIE *die)
{
	this->dwarf = dwarf;
	this->header = hdr;
	this->die = die;
}

Dwarf::Dwarf(const char *debugSymbols)
{
	// this->macho = new KernelMachO(debugSymbols);

	this->machoWithDebug = macho;

	this->dwarf = macho->getSegment("__DWARF");

	this->__debug_line = macho->getSection("__DWARF", "__debug_line");
	this->__debug_loc = macho->getSection("__DWARF", "__debug_loc");
	this->__debug_aranges = macho->getSection("__DWARF", "__debug_aranges");
	this->__debug_info = macho->getSection("__DWARF", "__debug_info");
	this->__debug_ranges = macho->getSection("__DWARF", "__debug_ranges");
	this->__debug_abbrev = macho->getSection("__DWARF", "__debug_abbrev");
	this->__debug_str = macho->getSection("__DWARF", "__debug_str");
	this->__apple_names = macho->getSection("__DWARF", "__apple_names");
	this->__apple_namespac = macho->getSection("__DWARF", "__apple_namespac");
	this->__apple_types = macho->getSection("__DWARF", "__apple_types");
	this->__apple_objc = macho->getSection("__DWARF", "__apple_objc");

	this->populateDebugSymbols();
}

Dwarf::Dwarf(MachO *macho, const char *debugSymbols)
{
	this->macho = macho;
	this->machoWithDebug = macho;

	this->dwarf = macho->getSegment("__DWARF");

	this->__debug_line = macho->getSection("__DWARF", "__debug_line");
	this->__debug_loc = macho->getSection("__DWARF", "__debug_loc");
	this->__debug_aranges = macho->getSection("__DWARF", "__debug_aranges");
	this->__debug_info = macho->getSection("__DWARF", "__debug_info");
	this->__debug_ranges = macho->getSection("__DWARF", "__debug_ranges");
	this->__debug_abbrev = macho->getSection("__DWARF", "__debug_abbrev");
	this->__debug_str = macho->getSection("__DWARF", "__debug_str");
	this->__apple_names = macho->getSection("__DWARF", "__apple_names");
	this->__apple_namespac = macho->getSection("__DWARF", "__apple_namespac");
	this->__apple_types = macho->getSection("__DWARF", "__apple_types");
	this->__apple_objc = macho->getSection("__DWARF", "__apple_objc");
}

DIE* Dwarf::getDebugInfoEntryByName(const char *name)
{
	return NULL;
}

DIE* getDebugInfoEntryByCode(uint64_t code)
{
	return NULL;
}

void Dwarf::populateDebugSymbols()
{
	// this->parseDebugAbbrev();
	// this->parseDebugInfo();
	// this->parseDebugLines();
	// this->parseDebugLocations();
	this->parseDebugRanges();
	// this->parseDebugAddressRanges();
}

void Dwarf::parseDebugAbbrev()
{
	MachO *macho = this->macho;

	Segment *dwarf = this->dwarf;

	Section *debug_info = this->__debug_info;
	Section *debug_abbrev = this->__debug_abbrev;
	Section *debug_str = this->__debug_str;

	uint8_t *debug_abbrev_begin = macho->getOffset(debug_abbrev->getOffset());
	uint8_t *debug_abbrev_end = macho->getOffset(debug_abbrev->getOffset() + debug_abbrev->getSize());

	uint8_t *debug_str_begin = macho->getOffset(debug_str->getOffset());
	uint8_t *debug_str_end = macho->getOffset(debug_str->getOffset() + debug_str->getSize());

	size_t debug_abbrev_size = debug_abbrev->getSize();

	uint32_t debug_abbrev_offset = 0;

	std::Array<DIE*> stack;

	uint64_t code = 0;

	enum DW_TAG tag = static_cast<enum DW_TAG>(0);

	while(debug_abbrev_offset < debug_abbrev->getSize() - sizeof(uint32_t))
	{
		if(code == 0 && static_cast<uint32_t>(tag) == 0)
		{
			code = Debug::ReadUleb128(debug_abbrev_begin + debug_abbrev_offset, debug_abbrev_end, &debug_abbrev_offset);

			tag = static_cast<enum DW_TAG>(Debug::ReadUleb128(debug_abbrev_begin + debug_abbrev_offset, debug_abbrev_end, &debug_abbrev_offset));

			if(tag == DW_TAG::compile_unit)
			{
				enum DW_CHILDREN children = static_cast<enum DW_CHILDREN>(Debug::ReadUleb128(debug_abbrev_begin + debug_abbrev_offset, debug_abbrev_end, &debug_abbrev_offset));

				char *name = DWTagToString(tag);

				DIE *die = new DIE(this, code, name, tag, children);

				stack.add(die);

				dies.add(die);

				MAC_RK_LOG("\n\n[%llu] DW_TAG = %s children = %u\n", code, name, static_cast<uint32_t>(children));
			}

		} else
		{
			uint64_t value;

			enum DW_AT attr = static_cast<enum DW_AT>(Debug::ReadUleb128(debug_abbrev_begin + debug_abbrev_offset, debug_abbrev_end, &debug_abbrev_offset));

			enum DW_FORM form = static_cast<enum DW_FORM>(Debug::ReadUleb128(debug_abbrev_begin + debug_abbrev_offset, debug_abbrev_end, &debug_abbrev_offset));

			if(static_cast<int>(attr) == 0 && static_cast<int>(form) == 0)
			{
				uint32_t tmp = debug_abbrev_offset;

				code = Debug::ReadUleb128(debug_abbrev_begin + debug_abbrev_offset, debug_abbrev_end, &debug_abbrev_offset);

				while(!code)
				{
					tmp = debug_abbrev_offset;

					code = Debug::ReadUleb128(debug_abbrev_begin + debug_abbrev_offset, debug_abbrev_end, &debug_abbrev_offset);
				}
		
				tag = static_cast<enum DW_TAG>(Debug::ReadUleb128(debug_abbrev_begin + debug_abbrev_offset, debug_abbrev_end, &debug_abbrev_offset));

				if(tag == DW_TAG::compile_unit)
				{
					tag = static_cast<enum DW_TAG>(0);
					code = 0;

					debug_abbrev_offset = tmp;

					continue;
				} else
				{
					enum DW_CHILDREN children = static_cast<enum DW_CHILDREN>(Debug::ReadUleb128(debug_abbrev_begin + debug_abbrev_offset, debug_abbrev_end, &debug_abbrev_offset));

					char *name = DWTagToString(tag);

					DIE *die = new DIE(this, code, name, tag, children);

					MAC_RK_LOG("\n\n[%llu] DW_TAG = %s children = %u\n", code, name, static_cast<uint32_t>(children));

					stack.add(die);

					dies.add(die);
				}

			} else
			{
				MAC_RK_LOG("\tDW_AT = %s 0x%llx DW_FORM = %s\n", DWAttrToString(attr), static_cast<uint32_t>(attr), DWFormToString(form));
				
				DIE *die = stack.get(stack.getSize() - 1);

				struct AttrAbbrev *ab = new AttrAbbrev;

				ab->tag = die->getTag();
				ab->children = die->getHasChildren();
				ab->code = die->getCode();

				ab->attr_spec.name = attr;
				ab->attr_spec.form = form;

				die->addAbbreviation(ab);

			}
		}
	}

	MAC_RK_LOG("\n\n");
}

DIE* Dwarf::getDebugInfoEntryByCode(uint64_t code)
{
	for(int i = 0; i < dies.getSize(); i++)
	{
		DIE *die = dies.get(i);

		if(die->getCode() == code)
		{
			return die;
		}
	}

	return NULL;
}

void Dwarf::parseDebugInfo()
{
	MachO *macho = this->macho;

	Segment *dwarf = this->dwarf;

	Section *debug_info = this->__debug_info;
	Section *debug_abbrev = this->__debug_abbrev;
	Section *debug_str = this->__debug_str;

	uint8_t *debug_info_begin = macho->getOffset(debug_info->getOffset());
	uint8_t *debug_info_end = macho->getOffset(debug_info->getOffset() + debug_info->getSize());

	uint8_t *debug_abbrev_begin = macho->getOffset(debug_abbrev->getOffset());
	uint8_t *debug_abbrev_end = macho->getOffset(debug_abbrev->getOffset() + debug_abbrev->getSize());

	uint8_t *debug_str_begin = macho->getOffset(debug_str->getOffset());
	uint8_t *debug_str_end = macho->getOffset(debug_str->getOffset() + debug_str->getSize());

	size_t debug_info_size = debug_info->getSize();
	size_t debug_abbrev_size = debug_abbrev->getSize();

	uint32_t debug_info_offset = 0;
	uint32_t debug_abbrev_offset = 0;

	struct CompilationUnit *compilationUnit = NULL;
	struct CompileUnitHeader *header = NULL;

	std::Array<DwarfDIE*> stack;

	uint32_t next_unit = 0;
	uint32_t consecutive_zeroes = 0;

	while(debug_info_offset < debug_info->getSize() - sizeof(uint32_t))
	{
		uint64_t value;

		bool new_compile_unit = false;

		if(header == NULL)
		{
			header = reinterpret_cast<struct CompileUnitHeader*>(debug_info_begin + debug_info_offset);

			debug_info_offset += sizeof(struct CompileUnitHeader);

			new_compile_unit = true;
		}

		uint64_t code = Debug::ReadUleb128(debug_info_begin + debug_info_offset, debug_info_end, &debug_info_offset);

		if(code == 0)
		{
			stack.remove(stack.getSize() - 1);

			if(stack.getSize() == 0)
			{
				header = NULL;
			}

			continue;
		}

		DIE *die = getDebugInfoEntryByCode(code);

		DwarfDIE *parent = stack.getSize() > 0 ? stack.get(stack.getSize() - 1) : NULL;

		DwarfDIE *dwarfDIE = new DwarfDIE(this, compilationUnit, die, parent);

		if(new_compile_unit)
		{
			compilationUnit = new CompilationUnit(this, header, die);

			this->compilationUnits.add(compilationUnit);

			new_compile_unit = false;
		}

		for(int i = 0; i < stack.getSize(); i++)
				MAC_RK_LOG("\t");

		MAC_RK_LOG("DW_TAG = %s depth = %zu\n", DWTagToString(die->getTag()), stack.getSize());

		uint64_t die_code = die->getCode();

		uint32_t attributes_count = die->getAttributesCount();

		for(int i = 0; i < attributes_count; i++)
		{
			struct Attribute *attribute = new Attribute;

			struct AttrAbbrev *ab = die->getAttribute(i);

			DW_AT attr = ab->attr_spec.name;
			DW_FORM form = ab->attr_spec.form;

			DW_TAG tag = ab->tag;
			DW_CHILDREN ch = ab->children;

			attribute->abbreviation.attr_spec.name = attr;
			attribute->abbreviation.attr_spec.form = form;
			attribute->abbreviation.tag = tag;
			attribute->abbreviation.children = ch;

			if(form == DW_FORM::flag_present)
			{
				continue;
			} else if(form == DW_FORM::block)
			{
				value = Debug::ReadUleb128(debug_info_begin + debug_info_offset, debug_info_end, &debug_info_offset);

				continue;
			} else if(form == DW_FORM::block1)
			{
				value = *reinterpret_cast<uint8_t*>(debug_info_begin + debug_info_offset);

				debug_info_offset += sizeof(uint8_t);

				debug_info_offset += value;

				continue;
			} else if(form == DW_FORM::block2)
			{
				value = *reinterpret_cast<uint16_t*>(debug_info_begin + debug_info_offset);

				debug_info_offset += sizeof(uint16_t);

				debug_info_offset += value;

				continue;
			} else if(form == DW_FORM::block4)
			{
				value = *reinterpret_cast<uint32_t*>(debug_info_begin + debug_info_offset);

				debug_info_offset += sizeof(uint32_t);

				debug_info_offset += value;

				continue;
			} else if(form == DW_FORM::indirect)
			{
				value = Debug::ReadUleb128(debug_info_begin + debug_info_offset, debug_info_end, &debug_info_offset);

				continue;
			} else if(form == DW_FORM::ref_udata)
			{
				value = Debug::ReadUleb128(debug_info_begin + debug_info_offset, debug_info_end, &debug_info_offset);

				continue;
			} else if(form == DW_FORM::udata)
			{
				value = Debug::ReadUleb128(debug_info_begin + debug_info_offset, debug_info_end, &debug_info_offset);

				continue;
			} else if(form == DW_FORM::sdata)
			{
				value = Debug::ReadSleb128(debug_info_begin + debug_info_offset, debug_info_end, &debug_info_offset);

				continue;
			} else if(form == DW_FORM::string)
			{
				uint64_t string_size = GetStringSize(debug_info_begin + debug_info_offset);

				debug_info_offset += string_size;

				continue;
			}else if(form == DW_FORM::exprloc)
			{
				value = Debug::ReadUleb128(debug_info_begin + debug_info_offset, debug_info_end, &debug_info_offset);

				debug_info_offset += value;

				continue;
			} else
			{
				size_t form_size = DWFormSize(form);

				switch(form_size)
				{
					case 0:
						break;
					case 1:
						value = *reinterpret_cast<uint8_t*>(debug_info_begin + debug_info_offset);

						break;
					case 2:
						value = *reinterpret_cast<uint16_t*>(debug_info_begin + debug_info_offset);

						break;
					case 4:
						value = *reinterpret_cast<uint32_t*>(debug_info_begin + debug_info_offset);

						break;
					case 8:
						value = *reinterpret_cast<uint64_t*>(debug_info_begin + debug_info_offset);

						break;
					default:


						break;
				}

				debug_info_offset += form_size;
			}

			attribute->value = value;

			dwarfDIE->addAttribute(attribute);

			for(int i = 0; i < stack.getSize(); i++)
				MAC_RK_LOG("\t");

			MAC_RK_LOG("\tDW_AT = %s value = 0x%llx\n", DWAttrToString(attr), value);
		}

		if(static_cast<bool>(die->getHasChildren()))
		{
			stack.add(dwarfDIE);
		}

		if(parent)
			parent->addChild(dwarfDIE);

		compilationUnit->addDebugInfoEntry(dwarfDIE);
	}
}

void Dwarf::parseDebugLines()
{
	MachO *macho = this->macho;

	Segment *dwarf = this->dwarf;

	Section *debug_line = this->__debug_line;

	uint8_t *debug_line_begin = macho->getOffset(debug_line->getOffset());
	uint8_t *debug_line_end = macho->getOffset(debug_line->getOffset() + debug_line->getSize());

	uint32_t debug_line_offset = 0;
	
	while(debug_line_offset < debug_line->getSize())
	{
		LineTable *lineTable = new LineTable(this->macho, this);

		struct LTPrologue prologue;
		struct LTStandardOpcodeLengths opcodes;

		memcpy(&prologue, debug_line_begin + debug_line_offset, sizeof(struct LTPrologue));

		uint32_t total_length = prologue.total_length;
		uint32_t prologue_length = prologue.prologue_length;

		uint8_t *prologue_end = reinterpret_cast<uint8_t*>(debug_line_begin + debug_line_offset + offsetof(struct LTPrologue, min_inst_length) + prologue_length);
		uint8_t *end = reinterpret_cast<uint8_t*>(debug_line_begin + debug_line_offset + total_length + sizeof(uint32_t));

		debug_line_offset += sizeof(struct LTPrologue);

		memcpy(&opcodes, debug_line_begin + debug_line_offset, sizeof(struct LTStandardOpcodeLengths));

		debug_line_offset += sizeof(struct LTStandardOpcodeLengths);

		bool source_file_names = false;

		while((debug_line_begin + debug_line_offset) < prologue_end)
		{
			if(source_file_names)
			{
				struct LTSourceFile *source_file = new LTSourceFile;

				char *source_file_name = reinterpret_cast<char*>(debug_line_begin + debug_line_offset);

				MAC_RK_LOG("Source File Name: %s\n", source_file_name);

				uint32_t string_size = GetStringSize(debug_line_begin + debug_line_offset);

				debug_line_offset += string_size + 1;

				lineTable->addSourceFile(source_file);

				memcpy(&source_file->metadata, debug_line_begin + debug_line_offset, sizeof(struct LTSourceFileMetadata));

				debug_line_offset += sizeof(struct LTSourceFileMetadata);

				if(*(debug_line_begin + debug_line_offset) == 0)
				{
					debug_line_offset++;

					break;
				}

			} else
			{
				char *include_directory = reinterpret_cast<char*>(debug_line_begin + debug_line_offset);

				MAC_RK_LOG("Include Directory: %s\n", include_directory);

				uint32_t string_size = GetStringSize(debug_line_begin + debug_line_offset);

				debug_line_offset += string_size + 1;

				lineTable->addIncludeDirectory(include_directory);

				if(*(debug_line_begin + debug_line_offset) == 0)
				{
					debug_line_offset++;

					source_file_names = true;
				}
			}
		}

		MAC_RK_LOG("%-20s %-6s %-6s %-6s %-4s %-13s %-13s\n", "Address", "Line", "Column", "File", "ISA", "Discriminator", "Flags");
		MAC_RK_LOG("%-20s %-6s %-6s %-6s %-4s %-13s %-13s\n", "--------------------", "--------", "------", "------", "----", "-------------", "-------------");

		struct Sequence *sequence = new Sequence;

		struct LTSourceFile *sourceFile = NULL;
		struct LTSourceLine *sourceLine = NULL;

		sourceLine = new LTSourceLine;

		memcpy(&sourceLine->state, &gInitialState, sizeof(struct LTStateMachine));

		sourceFile = lineTable->getSourceFile(sourceLine->state.file);

		sourceLine->source_file = sourceFile;

		uint8_t op_index = 0;

		while((debug_line_begin + debug_line_offset) < end)
		{
			uint8_t op = *(debug_line_begin + debug_line_offset);

			debug_line_offset++;

			if(op == 0)
			{
				uint64_t num_bytes = Debug::ReadUleb128(debug_line_begin + debug_line_offset, debug_line_end, &debug_line_offset);

				op = *reinterpret_cast<uint8_t*>(debug_line_begin + debug_line_offset);

				debug_line_offset++;

				num_bytes--;

				switch(static_cast<DW_LNE>(op))
				{
					case DW_LNE::end_sequence:
					;
					{
						sequence->sourceLines.add(sourceLine);

						struct LTSourceLine *newSourceLine = new LTSourceLine;

						memcpy(newSourceLine, sourceLine, sizeof(struct LTSourceLine));

						sourceLine = newSourceLine;

						sourceLine->state.end_sequence = 1;

						MAC_RK_LOG("0x%-20llx %-6lld %-8lld %-6u %-4u %-13u %-13s\n", sourceLine->state.address, sourceLine->state.line, sourceLine->state.column, sourceLine->state.file, sourceLine->state.isa, sourceLine->state.discriminator, SourceLineFlagsToString(sourceLine));

						memcpy(&sourceLine->state, &gInitialState, sizeof(struct LTStateMachine));

						sourceLine->state.discriminator = 0;
						sourceLine->state.basic_block = 0;
						sourceLine->state.prologue_end = 0;
						sourceLine->state.epilogue_begin = 0;
						sourceLine->state.end_sequence = 0;

						break;
					}
					case DW_LNE::set_address:
					;
					{
						uint64_t program_counter = *reinterpret_cast<uint64_t*>(debug_line_begin + debug_line_offset);

						sourceLine->state.address = program_counter;

						MAC_RK_LOG("0x%-20llx %-6lld %-8lld %-6u %-4u %-13u %-13s\n", sourceLine->state.address, sourceLine->state.line, sourceLine->state.column, sourceLine->state.file, sourceLine->state.isa, sourceLine->state.discriminator, SourceLineFlagsToString(sourceLine));

						sequence->sourceLines.add(sourceLine);

						struct LTSourceLine *newSourceLine = new LTSourceLine;

						memcpy(newSourceLine, sourceLine, sizeof(struct LTSourceLine));

						sourceLine = newSourceLine;

						break;
					}
					case DW_LNE::define_file:
					;
					{
						break;
					}
					case DW_LNE::set_discriminator:
					;
					{
						uint64_t discriminator = Debug::ReadUleb128(debug_line_begin + debug_line_offset, debug_line_end);

						sourceLine->state.discriminator = discriminator;

					 	// MAC_RK_LOG("0x%-20llx %-6lld %-8lld %-6u %-4u %-13u %-13s\n", sourceLine->state.address, sourceLine->state.line, sourceLine->state.column, sourceLine->state.file, sourceLine->state.isa, sourceLine->state.discriminator, SourceLineFlagsToString(sourceLine));

						break;
					}
					case DW_LNE::lo_user:
					;
					{
						break;
					}
					case DW_LNE::hi_user:
					;
					{
						break;
					}
				}

				debug_line_offset += num_bytes;
			} else if(op > 0 && op < 13)
			{
				switch(static_cast<DW_LNS>(op))
				{
					case DW_LNS::copy:
					{
						// MAC_RK_LOG("0x%-20llx %-6lld %-8lld %-6u %-4u %-13u %-13s\n", sourceLine->state.address, sourceLine->state.line, sourceLine->state.column, sourceLine->state.file, sourceLine->state.isa, sourceLine->state.discriminator, SourceLineFlagsToString(sourceLine));

						sourceLine->state.discriminator = 0;
						sourceLine->state.basic_block = 0;
						sourceLine->state.prologue_end = 0;
						sourceLine->state.epilogue_begin = 0;
						sourceLine->state.end_sequence = 0;

						break;
					}
					case DW_LNS::advance_pc:
					;
					{
						uint64_t program_counter = Debug::ReadUleb128(debug_line_begin + debug_line_offset, debug_line_end, &debug_line_offset);

						sourceLine->state.address += program_counter;
						sourceLine->state.prologue_end = 0;

						// MAC_RK_LOG("0x%-20llx %-6lld %-8lld %-6u %-4u %-13u %-13s\n", sourceLine->state.address, sourceLine->state.line, sourceLine->state.column, sourceLine->state.file, sourceLine->state.isa, sourceLine->state.discriminator, SourceLineFlagsToString(sourceLine));

						break;
					}

					case DW_LNS::advance_line:
					;
					{
						int64_t line = Debug::ReadSleb128(debug_line_begin + debug_line_offset, debug_line_end, &debug_line_offset);

						sourceLine->state.line += line;

						// MAC_RK_LOG("0x%-20llx %-6lld %-8lld %-6u %-4u %-13u %-13s\n", sourceLine->state.address, sourceLine->state.line, sourceLine->state.column, sourceLine->state.file, sourceLine->state.isa, sourceLine->state.discriminator, SourceLineFlagsToString(sourceLine));

						break;
					}
					case DW_LNS::set_file:
					;
					{
						uint64_t file = Debug::ReadUleb128(debug_line_begin + debug_line_offset, debug_line_end, &debug_line_offset);

						sourceLine->state.file = file;

						// MAC_RK_LOG("0x%-20llx %-6lld %-8lld %-6u %-4u %-13u %-13s\n", sourceLine->state.address, sourceLine->state.line, sourceLine->state.column, sourceLine->state.file, sourceLine->state.isa, sourceLine->state.discriminator, SourceLineFlagsToString(sourceLine));

						break;
					}
					case DW_LNS::set_column:
					;
					{
						uint64_t column = Debug::ReadUleb128(debug_line_begin + debug_line_offset, debug_line_end, &debug_line_offset);

						sourceLine->state.column = column;

						// MAC_RK_LOG("0x%-20llx %-6lld %-8lld %-6u %-4u %-13u %-13s\n", sourceLine->state.address, sourceLine->state.line, sourceLine->state.column, sourceLine->state.file, sourceLine->state.isa, sourceLine->state.discriminator, SourceLineFlagsToString(sourceLine));

						break;
					}
					case DW_LNS::negate_stmt:
					;
					{
						sourceLine->state.statement = ~sourceLine->state.statement;

						// MAC_RK_LOG("0x%-20llx %-6lld %-8lld %-6u %-4u %-13u %-13s\n", sourceLine->state.address, sourceLine->state.line, sourceLine->state.column, sourceLine->state.file, sourceLine->state.isa, sourceLine->state.discriminator, SourceLineFlagsToString(sourceLine));

						break;
					}
					case DW_LNS::set_basic_block:
					;
					{
						sourceLine->state.basic_block = 1;

						// MAC_RK_LOG("0x%-20llx %-6lld %-8lld %-6u %-4u %-13u %-13s\n", sourceLine->state.address, sourceLine->state.line, sourceLine->state.column, sourceLine->state.file, sourceLine->state.isa, sourceLine->state.discriminator, SourceLineFlagsToString(sourceLine));

						break;
					}
					case DW_LNS::const_add_pc:
					;
					{
						int64_t line_base = prologue.line_base;
						uint8_t line_range = prologue.line_range;

						uint8_t opcode_base = prologue.opcode_base;
						uint8_t min_inst_length = prologue.min_inst_length;

						uint64_t inc = min_inst_length * ((255 - opcode_base) / line_range);

						sourceLine->state.address += inc;

						break;
					}
					case DW_LNS::fixed_advance_pc:
					;
					{
						uint64_t program_counter = *reinterpret_cast<uint16_t*>(debug_line_begin + debug_line_offset, debug_line_end, &debug_line_offset);

						debug_line_offset += sizeof(uint16_t);

						sourceLine->state.address += program_counter;

						sourceLine->state.prologue_end = 0;

						// MAC_RK_LOG("0x%-20llx %-6lld %-8lld %-6u %-4u %-13u %-13s\n", sourceLine->state.address, sourceLine->state.line, sourceLine->state.column, sourceLine->state.file, sourceLine->state.isa, sourceLine->state.discriminator, SourceLineFlagsToString(sourceLine));

						break;
					}
					case DW_LNS::set_prologue_end:
					;
					{
						sourceLine->state.prologue_end = 1;

						// MAC_RK_LOG("0x%-20llx %-6lld %-8lld %-6u %-4u %-13u %-13s\n", sourceLine->state.address, sourceLine->state.line, sourceLine->state.column, sourceLine->state.file, sourceLine->state.isa, sourceLine->state.discriminator, SourceLineFlagsToString(sourceLine));

						break;
					}
					case DW_LNS::set_epilogue_begin:
					;
					{
						sourceLine->state.epilogue_begin = 1;

						// MAC_RK_LOG("0x%-20llx %-6lld %-8lld %-6u %-4u %-13u %-13s\n", sourceLine->state.address, sourceLine->state.line, sourceLine->state.column, sourceLine->state.file, sourceLine->state.isa, sourceLine->state.discriminator, SourceLineFlagsToString(sourceLine));

						break;
					}
					case DW_LNS::set_isa:
					;
					{
						uint64_t isa = Debug::ReadUleb128(debug_line_begin + debug_line_offset, debug_line_end, &debug_line_offset);

						sourceLine->state.isa = isa;

						// MAC_RK_LOG("0x%-20llx %-6lld %-8lld %-6u %-4u %-13u %-13s\n", sourceLine->state.address, sourceLine->state.line, sourceLine->state.column, sourceLine->state.file, sourceLine->state.isa, sourceLine->state.discriminator, SourceLineFlagsToString(sourceLine));

						break;
					}
				}

			} else
			{
				int64_t line_base = prologue.line_base;
				uint8_t line_range = prologue.line_range;

				uint8_t opcode_base = prologue.opcode_base;
				uint8_t min_inst_length = prologue.min_inst_length;

				int64_t address_change = ((op - opcode_base) / line_range) * min_inst_length;
				int64_t line_change = line_base + (op - opcode_base) % line_range;

				sourceLine->state.address += address_change;
				sourceLine->state.line += line_change;

				MAC_RK_LOG("0x%-20llx %-6lld %-8lld %-6u %-4u %-13u %-13s\n", sourceLine->state.address, sourceLine->state.line, sourceLine->state.column, sourceLine->state.file, sourceLine->state.isa, sourceLine->state.discriminator, SourceLineFlagsToString(sourceLine));
			
				sourceLine->state.prologue_end = 0;

				sequence->sourceLines.add(sourceLine);

				struct LTSourceLine *newSourceLine = new LTSourceLine;

				memcpy(newSourceLine, sourceLine, sizeof(struct LTSourceLine));

				sourceLine = newSourceLine;
			}
		}

		debug_line_offset = (end - debug_line_begin);

		this->lineTables.add(lineTable);
	}
}

void Dwarf::parseDebugLocations()
{
	MachO *macho = this->macho;

	Segment *dwarf = this->dwarf;

	Section *debug_loc = this->__debug_loc;

	uint8_t *debug_loc_begin = macho->getOffset(debug_loc->getOffset());
	uint8_t *debug_loc_end = macho->getOffset(debug_loc->getOffset() + debug_loc->getSize());

	uint32_t debug_loc_offset = 0;

	MAC_RK_LOG("0x%08x:\n", debug_loc_offset);

	struct LocationTableEntry *location_entry = new LocationTableEntry;

	location_entry->offset = debug_loc_offset;

	while(debug_loc_offset < debug_loc->getSize())
	{
		uint64_t value0 = *reinterpret_cast<uint64_t*>(debug_loc_begin + debug_loc_offset);

		uint64_t value1 = *reinterpret_cast<uint64_t*>(debug_loc_begin + debug_loc_offset + sizeof(uint64_t));

		if(value0 != 0 || value1 != 0)
		{
			debug_loc_offset += sizeof(uint64_t) * 2;

			uint16_t bytes = *reinterpret_cast<uint16_t*>(debug_loc_begin + debug_loc_offset);
		
			debug_loc_offset += sizeof(uint16_t);

			MAC_RK_LOG("\t(0x%016llx, 0x%016llx) ", value0, value1);

			for(int i = 0; i < bytes; i++)
			{
				uint8_t byte = *reinterpret_cast<uint8_t*>(debug_loc_begin + debug_loc_offset);

				location_entry->location_ops.add(static_cast<DW_OP>(byte));

				MAC_RK_LOG("0x%x ", byte);

				debug_loc_offset++;
			}

			MAC_RK_LOG("\n");

		} else if(value0 == -1ULL)
		{
			debug_loc_offset += sizeof(uint64_t) * 2;

			location_entry->kind = DW_LLE::base_address;
			
			location_entry->value0 = value1;
		} else
		{
			debug_loc_offset += sizeof(uint64_t) * 2;

			location_entry->kind = DW_LLE::end_of_list;

			this->locationTable.add(location_entry);

			location_entry = new LocationTableEntry;

			MAC_RK_LOG("0x%08x:\n", debug_loc_offset);

			location_entry->offset = debug_loc_offset;
		}
	}
}

void Dwarf::parseDebugRanges()
{
	MachO *macho = this->macho;

	Segment *dwarf = this->dwarf;

	Section *debug_ranges = this->__debug_ranges;

	uint8_t *debug_ranges_begin = macho->getOffset(debug_ranges->getOffset());
	uint8_t *debug_ranges_end = macho->getOffset(debug_ranges->getOffset() + debug_ranges->getSize());

	uint32_t debug_ranges_offset = 0;

	uint32_t current_ranges_offset = 0;

	RangeEntries *rangeEntries = new RangeEntries;
	
	while(debug_ranges_offset < debug_ranges->getSize())
	{
		uint64_t value0 = *reinterpret_cast<uint64_t*>(debug_ranges_begin + debug_ranges_offset);

		uint64_t value1 = *reinterpret_cast<uint64_t*>(debug_ranges_begin + debug_ranges_offset + sizeof(uint64_t));

		debug_ranges_offset += sizeof(uint64_t) * 2;

		if(value0 == 0 && value1 == 0)
		{
			MAC_RK_LOG("%08x <End of list>\n", current_ranges_offset);

			current_ranges_offset = debug_ranges_offset;

			this->ranges.add(rangeEntries);

			rangeEntries = new RangeEntries;
		} else
		{
			MAC_RK_LOG("%08x %016x %016x\n", current_ranges_offset, value0, value1);

			struct RangeEntry *range = new RangeEntry;

			range->offset = current_ranges_offset;
			range->value0 = value0;
			range->value1 = value1;

			rangeEntries->add(range);
		}
	}
}

void Dwarf::parseDebugAddressRanges()
{
	MachO *macho = this->macho;

	Segment *dwarf = this->dwarf;

	Section *debug_aranges = this->__debug_aranges;

	uint8_t *debug_aranges_begin = macho->getOffset(debug_aranges->getOffset());
	uint8_t *debug_aranges_end = macho->getOffset(debug_aranges->getOffset() + debug_aranges->getSize());

	uint32_t debug_aranges_offset = 0;

	uint32_t current_aranges_offset = 0;
	
	while(debug_aranges_offset < debug_aranges->getSize())
	{
		struct AddressRangeEntry *arange_entry = new AddressRangeEntry;

		struct AddressRangeHeader *address_range_header = &arange_entry->header;

		memcpy(address_range_header, debug_aranges_begin + debug_aranges_offset, sizeof(struct AddressRangeHeader));

		uint32_t length = address_range_header->length + sizeof(((struct AddressRangeHeader *)0)->length);

		uint32_t offset = debug_aranges_offset + sizeof(struct AddressRangeHeader);

		uint32_t segment_selector = *reinterpret_cast<uint32_t*>(debug_aranges_begin + offset);

		offset += sizeof(uint32_t);

		MAC_RK_LOG("Address Range Header: length = 0x%08x, version = 0x%04x, cu_offset = 0x%08x, addr_size = 0x%02x, seg_size = 0x%02x\n", address_range_header->length, address_range_header->version, address_range_header->offset, address_range_header->addr_size, address_range_header->seg_size);

		while(offset < debug_aranges_offset + length)
		{
			struct AddressRange *range = new AddressRange;

			uint64_t value0 = *reinterpret_cast<uint64_t*>(debug_aranges_begin + offset);

			uint64_t value1 = *reinterpret_cast<uint64_t*>(debug_aranges_begin + offset + sizeof(uint64_t));

			if(value0 != 0 || value1 != 0)
			{
				uint64_t address = *reinterpret_cast<uint64_t*>(debug_aranges_begin + offset);

				uint64_t size = *reinterpret_cast<uint64_t*>(debug_aranges_begin + offset + sizeof(uint64_t));

				range->start = address;
				range->end = address + size;

				MAC_RK_LOG("(0x%016llx, 0x%016llx)\n", address, address + size);

				arange_entry->ranges.add(range);
			}

			offset += (sizeof(uint64_t) * 2);
		}

		this->addressRanges.add(arange_entry);

		debug_aranges_offset += length;
	}
}


