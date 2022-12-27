#include "Dwarf.hpp"
#include "KernelMachO.hpp"

#include <assert.h>
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

	char *ret;
	
	asprintf(&ret, "unknown 0x%llx", static_cast<uint32_t>(tag));

	return strdup(ret);
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

	char *ret;
	
	asprintf(&ret, "unknown 0x%llx", static_cast<uint32_t>(attr));

	return strdup(ret);
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

	char *ret;
	
	asprintf(&ret, "unknown 0x%llx", static_cast<uint32_t>(form));

	return strdup(ret);
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

uint64_t Debug::GetStringSize(uint8_t *p)
{
	uint8_t *s = p;

	uint64_t size = 0;

	while(*s) size++;

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
			fprintf(stderr, "malformed uleb128\n");

			break;
		}

		uint64_t slice = *p & 0x7F;

		if(bit > 63)
		{
			fprintf(stderr, "uleb128 too big for uint64\n");

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
			fprintf(stderr, "malformed uleb128\n");

			break;
		}

		uint64_t slice = *p & 0x7F;

		if(bit > 63)
		{
			fprintf(stderr, "uleb128 too big for uint64\n");

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
			fprintf(stderr, "malformed sleb128\n");

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
			fprintf(stderr, "malformed sleb128\n");

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

CompilationUnit::CompilationUnit(Dwarf *dwarf, struct CompileUnitHeader *hdr, DIE *die)
{
	this->dwarf = dwarf;
	this->header = hdr;
	this->die = die;
}

Dwarf::Dwarf(const char *debugSymbols)
{
	this->macho = new KernelMachO(debugSymbols);

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

void Dwarf::populateDebugSymbols()
{
	this->parseDebugAbbrev();
	this->parseDebugInfo();
	this->parseDebugLocations();
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

	Array<DIE*> stack;

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

				printf("\n\n[%llu] DW_TAG = %s children = %u\n", code, name, static_cast<uint32_t>(children));
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

					printf("\n\n[%llu] DW_TAG = %s children = %u\n", code, name, static_cast<uint32_t>(children));

					stack.add(die);

					dies.add(die);
				}

			} else
			{
				printf("\tDW_AT = %s 0x%llx DW_FORM = %s\n", DWAttrToString(attr), static_cast<uint32_t>(attr), DWFormToString(form));
				
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

	struct CompileUnitHeader *header = NULL;

	Array<DIE*> stack;

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

		printf("offset = 0x%llx ", debug_info_offset);

		uint64_t code = Debug::ReadUleb128(debug_info_begin + debug_info_offset, debug_info_end, &debug_info_offset);

		printf("code = 0x%llx\n", code);

		if(code == 0)
		{
			stack.remove(stack.getSize() - 1);

			if(stack.getSize() == 0)
			{
				header = NULL;
			}

			printf("Stack size = 0x%llx\n", stack.getSize());

			continue;
		}

		printf("Stack size = 0x%llx\n", stack.getSize());

		DIE *die = getDebugInfoEntryByCode(code);

		assert(die);

		if(static_cast<bool>(die->getHasChildren()))
		{
			stack.add(die);
		}

		if(new_compile_unit)
		{
			CompilationUnit *compilationUnit = new CompilationUnit(this, header, die);

			this->compilationUnits.add(compilationUnit);

			new_compile_unit = false;
		}

		uint64_t die_code = die->getCode();

		uint32_t attributes_count = die->getAttributesCount();

		for(int i = 0; i < attributes_count; i++)
		{
			struct AttrAbbrev *ab = die->getAttribute(i);

			DW_AT attr = ab->attr_spec.name;
			DW_FORM form = ab->attr_spec.form;

			if(form == DW_FORM::flag_present)
			{
				continue;
			}

			if(form == DW_FORM::block)
			{
				value = Debug::ReadUleb128(debug_info_begin + debug_info_offset, debug_info_end, &debug_info_offset);

				printf("\tform = 0x%llx\n", static_cast<uint32_t>(form));
				printf("\tvalue = 0x%llx\n", value);

				continue;
			}

			if(form == DW_FORM::block1)
			{
				value = *reinterpret_cast<uint8_t*>(debug_info_begin + debug_info_offset);

				debug_info_offset += sizeof(uint8_t);

				printf("\tform = 0x%llx\n", static_cast<uint32_t>(form));
				printf("\tvalue = 0x%llx\n", value);

				debug_info_offset += value;

				continue;
			}

			if(form == DW_FORM::block2)
			{
				value = *reinterpret_cast<uint16_t*>(debug_info_begin + debug_info_offset);

				debug_info_offset += sizeof(uint16_t);

				printf("\tform = 0x%llx\n", static_cast<uint32_t>(form));
				printf("\tvalue = 0x%llx\n", value);

				debug_info_offset += value;

				continue;
			}

			if(form == DW_FORM::block4)
			{
				value = *reinterpret_cast<uint32_t*>(debug_info_begin + debug_info_offset);

				debug_info_offset += sizeof(uint32_t);

				printf("\tform = 0x%llx\n", static_cast<uint32_t>(form));
				printf("\tvalue = 0x%llx\n", value);

				debug_info_offset += value;

				continue;
			}

			if(form == DW_FORM::indirect)
			{
				value = Debug::ReadUleb128(debug_info_begin + debug_info_offset, debug_info_end, &debug_info_offset);

				printf("\tform = 0x%llx\n", static_cast<uint32_t>(form));
				printf("\tvalue = 0x%llx\n", value);

				continue;
			}

			if(form == DW_FORM::ref_udata)
			{
				value = Debug::ReadUleb128(debug_info_begin + debug_info_offset, debug_info_end, &debug_info_offset);

				printf("\tform = 0x%llx\n", static_cast<uint32_t>(form));
				printf("\tvalue = 0x%llx\n", value);

				continue;
			}

			if(form == DW_FORM::udata)
			{
				value = Debug::ReadUleb128(debug_info_begin + debug_info_offset, debug_info_end, &debug_info_offset);

				printf("\tform = 0x%llx\n", static_cast<uint32_t>(form));
				printf("\tvalue = 0x%llx\n", value);

				continue;
			}

			if(form == DW_FORM::sdata)
			{
				printf("debug_info_offset = 0x%llx\n", debug_info_offset);

				value = Debug::ReadSleb128(debug_info_begin + debug_info_offset, debug_info_end, &debug_info_offset);

				printf("debug_info_offset = 0x%llx\n", debug_info_offset);

				printf("\tform = 0x%llx\n", static_cast<uint32_t>(form));
				printf("\tvalue = 0x%llx\n", value);

				continue;
			}

			if(form == DW_FORM::string)
			{
				uint64_t string_size = GetStringSize(debug_info_begin + debug_info_offset);

				printf("\tform = 0x%llx\n", static_cast<uint32_t>(form));
				printf("\tvalue = 0x%llx\n", value);

				debug_info_offset += string_size;

				continue;
			}

			if(form == DW_FORM::exprloc)
			{
				value = Debug::ReadUleb128(debug_info_begin + debug_info_offset, debug_info_end, &debug_info_offset);

				printf("\tform = 0x%llx\n", static_cast<uint32_t>(form));
				printf("\tvalue = 0x%llx\n", value);

				debug_info_offset += value;

				continue;
			}

			size_t form_size = DWFormSize(form);
			
			printf("\tform = 0x%llx form_size = 0x%llx\n", static_cast<uint32_t>(form), form_size);

			assert(form_size != 0);

			switch(form_size)
			{
				case 0:
					break;
				case 1:
					value = *reinterpret_cast<uint8_t*>(debug_info_begin + debug_info_offset);

					printf("\tvalue = 0x%llx\n", value);

					break;
				case 2:
					value = *reinterpret_cast<uint16_t*>(debug_info_begin + debug_info_offset);

					printf("\tvalue = 0x%llx\n", value);

					break;
				case 4:
					value = *reinterpret_cast<uint32_t*>(debug_info_begin + debug_info_offset);

					printf("\tvalue = 0x%llx\n", value);

					break;
				case 8:
					value = *reinterpret_cast<uint64_t*>(debug_info_begin + debug_info_offset);

					printf("\tvalue = 0x%llx\n", value);

					break;
				default:


					break;
			}

			debug_info_offset += form_size;

			if(value)
			{

			}
		}
	}
}

void Dwarf::parseDebugLocations()
{

}


