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
	return macho->getObjCMetadata() ? new SwiftMetadata(macho, macho->getObjCMetadata()) : NULL;
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
	this->enumerateTypes();
}

void SwiftMetadata::enumerateTypes()
{
	Section *types = this->getTypes();

	uint8_t *swift_types_begin = (*macho)[types->getOffset()];
	uint8_t *swift_types_end = (*macho)[types->getOffset() + types->getSize()];

	uint32_t swift_types_offset = 0;

	while(swift_types_offset < types->getSize())
	{
		struct Type *type;

		mach_vm_address_t type_address;

		int64_t type_offset;

		type_offset = *reinterpret_cast<int32_t*>((*macho)[swift_types_offset]);

		type_address += type_offset;

		struct TypeDescriptor *descriptor = reinterpret_cast<struct TypeDescriptor*>((*macho)[type_offset]);

		type = this->parseTypeDescriptor(descriptor);

		swift_types_offset += sizeof(int32_t);
	}
}

struct Type* SwiftMetadata::parseTypeDescriptor(struct TypeDescriptor *typeDescriptor)
{
	struct Type *type;

	struct TypeDescriptor *descriptor;

	struct FieldDescriptor *fieldDescriptor;

	descriptor = typeDescriptor;

	int32_t field_descriptor_offset = *reinterpret_cast<int32_t*>(&typeDescriptor->field_descriptor);

	mach_vm_address_t field_descriptor_address = reinterpret_cast<mach_vm_address_t>(&typeDescriptor->field_descriptor) + field_descriptor_offset;

	fieldDescriptor = reinterpret_cast<struct FieldDescriptor*>(field_descriptor_address);

	type = NULL;

	switch(fieldDescriptor->kind)
	{
		case FDK_Struct:
			{
				struct Struct *structure = new Struct;

				memcpy(&structure->descriptor, typeDescriptor, sizeof(structure->descriptor));

				type = dynamic_cast<struct Type*>(structure);
			}

			break;
		case FDK_Class:
			{
				struct Class *cls = new Class;

				memcpy(&cls, typeDescriptor, sizeof(struct Class));

				type = dynamic_cast<struct Type*>(cls);
			}

			break;
		case FDK_Enum:
			{
				struct Enum *enumeration = new Enum;

				memcpy(&enumeration->descriptor, typeDescriptor, sizeof(struct Enum));

				type = dynamic_cast<struct Type*>(enumeration);
			}

			break;
		case FDK_MultiPayloadEnum:
			break;
		case FDK_Protocol:
			{
				struct Protocol *protocol = new Protocol;

				memcpy(&protocol->descriptor, typeDescriptor, sizeof(struct Protocol));

				type = dynamic_cast<struct Type*>(protocol);
			}

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
		this->parseFieldDescriptor(type, fieldDescriptor);

	return type;
}

mach_vm_address_t SwiftMetadata::getTypeMetadata(struct TypeDescriptor *typeDescriptor)
{
/*
#ifdef __arm64__

	using namespace Arch::arm64;

	mach_vm_address_t add = Arch::arm64::PatchFinder::step64(libobjc, start, 0x100,(bool (*)(uint32_t*))is_add_reg, -1, -1);

	mach_vm_address_t xref = Arch::arm64::PatchFinder::stepBack64(libobjc, add, 0x100,(bool (*)(uint32_t*))is_adrp, -1, -1);

	adr_t adrp = *reinterpret_cast<adr_t*>(xref);

	add_imm_t add_imm = *reinterpret_cast<add_imm_t*>(xref + 0x4);

	selectors = (xref & ~0xFFF) + ((((adrp.immhi << 2) | adrp.immlo)) << 12) + (add_imm.sh ? (add_imm.imm << 12) : add_imm.imm);

	return (selectors - start) + method_getName;

#elif __x86_64__

	using namespace Arch::x86_64;

	cs_insn insn;

	mach_vm_address_t add = Arch::x86_64::PatchFinder::step64(libobjc, start, 0x100, "add", NULL);

	mach_vm_address_t mov = Arch::x86_64::PatchFinder::stepBack64(libobjc, add, 0x100, "mov", NULL);

	Arch::x86_64::disassemble(mov, Arch::x86_64::MaxInstruction, &insn);

	mach_vm_address_t selectors = insn.detail.x86->operands[1].mem.disp + mov;

	return selectors;

#endif
*/
}

void SwiftMetadata::parseFieldDescriptor(struct Type *type, struct FieldDescriptor *fieldDescriptor)
{
	struct Fields *fields = new Fields;

	mach_vm_address_t field_start = reinterpret_cast<mach_vm_address_t>(fieldDescriptor) + sizeof(struct FieldDescriptor);

	fields->descriptor = fieldDescriptor;

	for(int i = 0; i < fieldDescriptor->num_fields; i++)
	{
		struct Field *field = new Field;

		memcpy(&field->record, reinterpret_cast<struct FieldRecord*>(field_start) + i, sizeof(struct FieldRecord));

		field->name = "";
		field->mangled_name = "";
		field->demangled_name = "";

		fields->records.add(field);

		type->field = field;
	}
}


}