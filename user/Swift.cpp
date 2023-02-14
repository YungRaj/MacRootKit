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

	this->typeref = this->macho->getSection("__TEXT", "__swift_typeref");
	this->entry = this->macho->getSection("__TEXT", "__swift_entry");
	this->builtin = this->macho->getSection("__TEXT", "__swift_builtin");
	this->reflstr = this->macho->getSection("__TEXT", "__swift_refstr");
	this->fieldmd = this->macho->getSection("__TEXT", "__swift_fieldmd");
	this->assocty = this->macho->getSection("__TEXT", "__swift_assocty");
	this->proto = this->macho->getSection("__TEXT", "__swift_proto");
	this->types = this->macho->getSection("__TEXT", "__swift_types");
	this->protos = this->macho->getSection("__TEXT", "__swift_protos");
	this->capture = this->macho->getSection("__TEXT", "__swift_capture");
	this->mpenum = this->macho->getSection("__TEXT", "__swift_mpenum");
}

void SwiftMetadata::parseSwift()
{
	this->parseTypes();
}

void SwiftMetadata::parseTypes()
{
	Section *types = this->getTypes();

	uint8_t *swift_types_begin = macho->getOffset(types->getOffset());
	uint8_t *swift_types_end = macho->getOffset(types->getOffset() + types->getSize());

	uint32_t swift_types_offset = 0;

	while(swift_types_offset < types->getSize())
	{
		mach_vm_address_t type_address;

		int64_t type_offset;

		type_offset = *reinterpret_cast<int32_t*>(macho->getOffset(swift_types_offset));

		type_address += type_offset;

		uint64_t type_offset = macho->addressToOffset(type_address);

		struct TypeDescriptor *descriptor = reinterpret_cast<struct TypeDescriptor*>(macho->getOffset(type_offset));

		this->parseType(descriptor);

		swift_types_offset += sizeof(int32_t);
	}
}

void SwiftMetadata::parseTypeDescriptor(struct TypeDescriptor *type)
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
}

void SwiftMetadata::parseFieldDescriptor(struct FieldDescriptor *field)
{

}

}