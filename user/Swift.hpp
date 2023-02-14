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
	// Swift Runtime functions
	// https://github.com/apple/swift/blob/main/docs/Runtime.md

	// Swift Type Metadata
	// https://github.com/apple/swift/blob/main/docs/ABI/TypeMetadata.rst

	/// Non-type metadata kinds have this bit set.
	const unsigned MetadataKindIsNonType = 0x400;

	/// Non-heap metadata kinds have this bit set.
	const unsigned MetadataKindIsNonHeap = 0x200;

	// The above two flags are negative because the "class" kind has to be zero,
	// and class metadata is both type and heap metadata.

	/// Runtime-private metadata has this bit set. The compiler must not statically
	/// generate metadata objects with these kinds, and external tools should not
	/// rely on the stability of these values or the precise binary layout of
	/// their associated data structures.
	const unsigned MetadataKindIsRuntimePrivate = 0x100;

	/// Kinds of context descriptor.
	enum class ContextDescriptorKind : uint8_t
	{
	  /// This context descriptor represents a module.
	  Module = 0,
	  
	  /// This context descriptor represents an extension.
	  Extension = 1,
	  
	  /// This context descriptor represents an anonymous possibly-generic context
	  /// such as a function body.
	  Anonymous = 2,

	  /// This context descriptor represents a protocol context.
	  Protocol = 3,
	  
	  /// This context descriptor represents an opaque type alias.
	  OpaqueType = 4,

	  /// First kind that represents a type of any sort.
	  Type_First = 16,
	  
	  /// This context descriptor represents a class.
	  Class = Type_First,
	  
	  /// This context descriptor represents a struct.
	  Struct = Type_First + 1,
	  
	  /// This context descriptor represents an enum.
	  Enum = Type_First + 2,
	  
	  /// Last kind that represents a type of any sort.
	  Type_Last = 31,
	};

	enum MetadataKind : uint16_t
	{
		MK_Class 						 = 0x0,
		MK_Struct						 = 0x200,
		MK_Enum 						 = 0x201,
		MK_Optional						 = 0x202,
		MK_ForeignClass					 = 0x203,
		MK_ForeignReferenceType 		 = 0x204,
		MK_Opaque 						 = 0x300,
		MK_Tuple 						 = 0x301,
		MK_Function 					 = 0x302,
		MK_Existential 					 = 0x303,
		MK_Metatype 					 = 0x304,
		MK_ObjCClassWrapper				 = 0x305,
		MK_ExistentialMetatype			 = 0x306,
		MK_ExtendedExistential			 = 0x307,
		MK_HeapLocalVariable 			 = 0x400,
		MK_HeapGenericLocalVariable		 = 0x500,
		MK_ErrorObject					 = 0x501,
		MK_Task							 = 0x502,
		MK_Job 							 = 0x503,
		MK_LastEnumerated				 = 0x7FF,
	};

	enum FieldDescriptorKind : uint16_t
	{
		FDK_Struct				 = 0,
		FDK_Class 				 = 1,
		FDK_Enum 				 = 2,
		FDK_MultiPayloadEnum	 = 3,
		FDK_Protocol			 = 4,
		FDK_ClassProtocol		 = 5,
		FDK_ObjCProtocol		 = 6,
		FDK_ObjCClass			 = 7,
	};

	struct ValueMetadata
	{
		uint64_t kind;
		uint64_t description;
	};

	struct ValueWitnessTable
	{
		mach_vm_address_t initializeBufferWithCopyOfBuffer;
		mach_vm_address_t destroy;
		mach_vm_address_t initializeWithCopy;
		mach_vm_address_t assignWithCopy;
		mach_vm_address_t initializeWithTake;
		mach_vm_address_t assignWithTake;
		mach_vm_address_t getEnumTagSinglePayload;
		mach_vm_address_t storeEnumTagSinglePayload;

		size_t size;
		size_t stride;
		
		uint64_t flags;
		uint64_t extra_inhabitant_count;
	};

	struct AnonymousContextDescriptor
	{
		uint32_t flags;
		uint32_t parent;
	};

	struct ModuleDescriptor
	{
		uint32_t flags;
		uint32_t parent;
		uint32_t name;
	};

	struct Module
	{

	};

	struct ProtocolDescriptor
	{
		uint32_t flags;
		uint32_t name;
		uint32_t num_requirements_in_signature;
		uint32_t num_requirements;
		uint32_t associated_type_names;
	};

	struct Protocol
	{
		struct ProtocolDescriptor descriptor;
	};

	struct Type
	{
		struct ModuleDescriptor *module;

		enum MetadataKind kind;

		char *name;
	};

	struct TypeDescriptor
	{
		uint32_t flags;
		int32_t parent;
		int32_t name;
		int32_t access_function;
		uint32_t field_descriptor;
	};

	struct ClassDescriptor
	{
		struct TypeDescriptor type;

		uint32_t super_class_type;

		uint32_t metadata_negative_size_in_words;
		uint32_t metadata_positive_size_in_words;

		uint32_t num_immediate_members;
		uint32_t num_fields;
	};

	struct Class : Type
	{
		struct ClassDescriptor descriptor;

		ObjectiveC::ObjCClass *isa;
	};

	struct StructDescriptor
	{
		struct TypeDescriptor type;

		uint32_t num_fields;
		uint32_t field_offset_vector_offset;
	};

	struct Struct : Type
	{
		struct StructDescriptor descriptor;
	};

	struct EnumDescriptor
	{
		struct TypeDescriptor type;

		uint32_t num_payload_cases_and_payload_size_offset;
		uint32_t num_empty_cases;
	};

	struct Enum : Type
	{
		struct EnumDescriptor descriptor;
	};

	struct FieldDescriptor
	{
		uint32_t type_ref;
		uint32_t mangled_type_name;
		uint16_t kind;
		uint16_t field_record_size;
		uint32_t num_fields;
	};

	struct FieldRecord
	{
		uint32_t flags;
		uint32_t mangled_type_name;
		uint32_t field_name;
		uint32_t field_record;
	};

	struct Field
	{
		struct FieldDescriptor field_;

		std::Array<struct FieldRecord*> records;
	};

	class SwiftMetadata
	{
		public:
			explicit SwiftMetadata(MachO *macho, ObjectiveC::ObjCData *objc) { this->macho = macho; this->objc = objc; this->populateSections(); this->parseSwift(); }
			explicit SwiftMetadata(MachO *macho, ObjectiveC::ObjCData *objc, Segment *text) : SwiftMetadata(macho, objc) { this->text = text; this->populateSections(); this->parseSwift(); }

			std::Array<Type*>* getAllTypes() { return &all_types; }

			ObjectiveC::ObjCData* getObjCMetaData() { return objc; }

			Segment* getTextSegment() { return text; }

			Section* getTypeRef() { return typeref; }
			Section* getEntry() { return entry; }
			Section* getBuiltIn() { return builtin; }
			Section* getReflStr() { return reflstr; }
			Section* getFieldMd() { return fieldmd; }
			Section* getAssocty() { return assocty; }
			Section* getProto() { return proto; };
			Section* getTypes() { return types; }
			Section* getProtos() { return protos; }
			Section* getCapture() { return capture; }
			Section* getMpenum() { return mpenum; }

			void setText(Segment *segment) { this->text = segment; this->populateSections(); this->parseSwift(); }

			void populateSections();

			void parseSwift();

			void parseTypes();

			void parseTypeDesciptor(struct TypeDescriptor *type);
			void parseFieldDescriptor(struct FieldDescriptor *field);

		private:
			MachO *macho;

			ObjectiveC::ObjCData *objc;

			Segment *text;

			std::Array<Type*> all_types;

			Section *typeref;
			Section *entry;
			Section *builtin;
			Section *reflstr;
			Section *fieldmd;
			Section *assocty;
			Section *proto;
			Section *types;
			Section *protos;
			Section *capture;
			Section *mpenum;
	};
};

#endif