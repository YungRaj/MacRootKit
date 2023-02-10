#ifndef __SWIFT_HPP_
#define __SWIFT_HPP_

#include "ObjC.hpp"

#include "Array.hpp"
#include "Dictionary.hpp"

class MachO;

class Segment;
class Section;

namespace Swift
{
	class Class
	{
		public:

		private:
	};

	class Protocol
	{
		public:

		private:
	}

	class Extension
	{
		public:

		private:
	};

	enum MetadataKind
	{
		MK_Class 						 0x0,
		MK_Struct						 0x200,
		MK_Enum 						 0x201,
		MK_Optional						 0x202
		MK_ForeignClass					 0x203,
		MK_ForeignReferenceType 		 0x204,
		MK_Opaque 						 0x300,
		MK_Tuple 						 0x301,
		MK_Function 					 0x302,
		MK_Existential 					 0x303,
		MK_Metatype 					 0x304,
		MK_ObjCClassWrapper				 0x305,
		MK_ExistentialMetatype			 0x306,
		MK_ExtendedExistential			 0x307,
		MK_HeapLocalVariable 			 0x400,
		MK_HeapGenericLocalVariable		 0x500,
		MK_ErrorObject					 0x501,
		MK_Task							 0x502,
		MK_Job 							 0x503,
		MK_LastEnumerated				 0x7FF,
	};

	enum FieldDescriptorKind
	{
		FDK_Struct				0,
		FDK_Class 				1,
		FDK_Enum 				2,
		FDK_MultiPayloadEnum	3,
		FDK_Protocol			4,
		FDK_ClassProtocol		5,
		FDK_ObjCProtocol		6,
		FDK_ObjCClass			7,
	};

	struct ModuleDescriptor
	{

	};

	struct TypeDescriptor
	{
		struct ModuleDescriptor *module;
		
		MetadataKind kind;

		char *name;
	};

	struct ClassDescriptor : TypeDescriptor
	{
		ObjCClass *isa;
	};

	struct StructDescriptor : TypeDescriptor
	{

	};

	struct EnumDescriptor : TypeDescriptor
	{

	};

	struct FieldDescriptor
	{


		std::Array<FieldRecord*> records;
	};

	struct FieldRecord
	{

	};


	class SwiftMetadata
	{
		public:
			explicit SwiftMetadata(MachO *macho, ObjCMetaData *objc);
			explicit SwiftMetadata(MachO *macho, ObjCMetaData *objc, Segment *swift);

			Segment* getSwiftSegment() { return swift; }

			Section* getTypeRef() { return typeref; }
			Section* getEntry() { return entry; }
			Section* getBuiltIn() { return builtin; }
			Section* getReflStr() { return reflstr; }
			Section* getFieldMd() { return fieldmd; }
			Section* getAssocty() { return assocty; }
			Section* getProto() { return proto; };
			Section* getTypes() { return types; }
			Section* getCapture() { return capture; }
			Section* getMpenum() { return mpenum; }

		private:
			MachO *macho;

			Segment *swift;

			Section *typeref;
			Section *entry;
			Section *builtin
			Section *reflstr;
			Section *fieldmd;
			Section *assocty;
			Section *proto;
			Section *types;
			Section *capture;
			Section *mpenum;
	};
};

#endif