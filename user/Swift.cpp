#include "ObjC.hpp"

#include "MachO.hpp"
#include "UserMachO.hpp"

#include "PAC.hpp"

#include "Swift.hpp"

#include <assert.h>
#include <string.h>

namespace Swift
{

SwiftMetadata* parseSwift(mrk::UserMachO *macho)
{
	return new SwiftMetadata(macho);
}

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

struct Type* SwiftMetadata::parseTypeDescriptor(struct TypeDescriptor *typeDescriptor)
{
	struct Type *type;

	struct TypeDescriptor *descriptor;

	struct FieldDescriptor *fieldDescriptor;

	descriptor = type;

	int32_t field_descriptor_offset = reinterpret_cast<int32_t*>(&typeDescriptor->field_descriptor);

	mach_vm_address_t field_descriptor_address = reinterpret_cast<mach_vm_address_t>(&typeDescriptor->field_descriptor) + field_descriptor_offset;

	fieldDescriptor = reinterpret_cast<struct FieldDescriptor*>(field_descriptor_address);

	type = NULL;

	switch(field_descriptor->kind)
	{
		case FDK_Struct:
			struct Struct *structure = new Struct;

			memcpy(&structure->descriptor, &type, sizeof(structure->descriptor));

			struct TypeDescriptor *descriptor = &structure->descriptor.type;

			type = dynamic_cast<struct Type*>(structure);

			break;
		case FDK_Class:
			struct Class *cls = new Class;

			memcpy(&cls->descriptor, &type, sizeof(cls->descriptor));

			struct TypeDescriptor *descriptor = &cls->descriptor.type;

			type = dynamic_cast<struct Type*>(cls);

			break;
		case FDK_Enum:
			struct Enum *enumeration = new Enum;

			memcpy(&enumeration->descriptor, &type, sizeof(enumeration->descriptor));

			struct TypeDescriptor *descriptor = &enumeration->descriptor.type;

			type = dynamic_cast<struct Type*>(enumeration);

			break;
		case FDK_MultiPayloadEnum:
			break;
		case FDK_Protocol:
			struct Protocol *protocol = new Protocol;

			memcpy(&protocol->descriptor, &type, sizeof(protocol->descriptor));

			struct TypeDescriptor *descriptor = &protocol->descriptor.type;

			type = dynamic_cast<struct Type*>(protocol);

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

	if(type)
		this->parseFieldDescriptor(type, field);

	return type;
}

mach_vm_address_t SwiftMetadata::getTypeMetadata(struct TypeDescriptor *typeDescriptor)
{
	
	
	return 0;
}

void SwiftMetadata::parseFieldDescriptor(struct Type *type, struct FieldDescriptor *fieldDescriptor)
{
	struct Fields *fields = new Fields;

	mach_vm_address_t field_start = reinterpret_cast<mach_vm_address_t>(fieldDescriptor) + sizeof(struct FieldDescriptor);

	field->descriptor = fieldDescriptor;

	for(int i = 0; i < fieldDescriptor->num_fields; i++)
	{
		struct Field *field = new Field;

		memcpy(&field->record, field_start + i * sizeof(struct FieldRecord), sizeof(struct FieldRecord));

		field->name = "";
		field->mangled_name = "";
		field->demangled_name = "";

		fields->records.add(field);
	}
}


}