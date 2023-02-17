#include "ObjC.hpp"

#include "MachO.hpp"
#include "UserMachO.hpp"

#include "PAC.hpp"

#include "Swift.hpp"

#include <assert.h>
#include <string.h>

namespace Swift
{

void SwiftMetadata::populateSections()
{
	if(!this->text)
		this->text = macho->getSegment("__TEXT");

	this->typeref = this->macho->getSection("__TEXT", "__swift5_typeref");
	this->entry = this->macho->getSection("__TEXT", "__swift5_entry");
	this->builtin = this->macho->getSection("__TEXT", "__swift5_builtin");
	this->reflstr = this->macho->getSection("__TEXT", "__swift5_refstr");
	this->fieldmd = this->macho->getSection("__TEXT", "__swift5_fieldmd");
	this->assocty = this->macho->getSection("__TEXT", "__swift5_assocty");
	this->proto = this->macho->getSection("__TEXT", "__swift5_proto");
	this->types = this->macho->getSection("__TEXT", "__swift5_types");
	this->protos = this->macho->getSection("__TEXT", "__swift5_protos");
	this->capture = this->macho->getSection("__TEXT", "__swift5_capture");
	this->mpenum = this->macho->getSection("__TEXT", "__swift5_mpenum");
}

void SwiftMetadata::parseSwift()
{
	this->parseTypes();
}

void SwiftMetadata::enumerateTypes()
{
	Section *types = this->getTypes();

	uint8_t *swift_types_begin = macho->getOffset(types->getOffset());
	uint8_t *swift_types_end = macho->getOffset(types->getOffset() + types->getSize());

	uint32_t swift_types_offset = 0;

	while(swift_types_offset < types->getSize())
	{
		struct Type *type;

		mach_vm_address_t type_address;

		int64_t type_offset;

		type_offset = *reinterpret_cast<int32_t*>(macho->getOffset(swift_types_offset));

		type_address += type_offset;

		uint64_t type_offset = macho->addressToOffset(type_address);

		struct TypeDescriptor *descriptor = reinterpret_cast<struct TypeDescriptor*>(macho->getOffset(type_offset));

		type = this->parseType(descriptor);

		swift_types_offset += sizeof(int32_t);
	}
}

struct Type* SwiftMetadata::parseTypeDescriptor(struct TypeDescriptor *type)
{
	struct Type *type;

	struct TypeDescriptor *descriptor;

	struct FieldDescriptor field_descriptor;

	descriptor = type;

	int32_t field_descriptor_offset = reinterpret_cast<int32_t*>(&descriptor->field_descriptor);

	mach_vm_address_t field_descriptor_address = reinterpret_cast<mach_vm_address_t>(&descriptor->field_descriptor) + field_descriptor_offset;

	field_descriptor = reinterpret_cast<struct FieldDescriptor*>(field_descriptor_address);

	switch(field_descriptor->kind)
	{
		case FDK_Struct:
			struct Struct *structure = new Struct;

			

			type = dynamic_cast<struct Type*>(structure);

			break;
		case FDK_Class:
			break;
		case FDK_Enum:
			break;
		case FDK_MultiPayloadEnum:
			break;
		case FDK_Protocol:
			break;
		case FDK_ClassProtocol:
			break;
		case FDK_ObjCProtocol:
			break;
		case FDK_ObjCClass:
			break;
		default:
			break;
	}

	return type;
}

struct Field* SwiftMetadata::parseFieldDescriptor(struct FieldDescriptor *field)
{

}

void SwiftMetadata::parseFieldRecord(struct FieldRecord *record)
{

}

}