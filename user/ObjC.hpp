#ifndef __OBJC_H_
#define __OBJC_H_

#include <mach/mach_types.h>

#include "Array.hpp"
#include "PAC.hpp"

class Segment;
class Section;

class MachO;

namespace mrk
{
	class UserMachO;
}

// Two bits of entsize are used for fixup markers.
// Reserve the top half of entsize for more flags. We never
// need entry sizes anywhere close to 64kB.
//
// Currently there is one flag defined: the small method list flag,
// method_t::smallMethodListFlag. Other flags are currently ignored.
// (NOTE: these bits are only ignored on runtimes that support small
// method lists. Older runtimes will treat them as part of the entry
// size!)


// This flag is ORed into method list pointers to indicate that the list is
// a big list with signed pointers. Use a bit in TBI so we don't have to
// mask it out to use the pointer.
#ifdef __arm64e__

static const uintptr_t bigSignedMethodListFlag = 0x8000000000000000;

#else

static const uintptr_t bigSignedMethodListFlag = 0x0;

#endif

static const uint32_t smallMethodListFlag = 0x80000000;

// We don't use this currently, but the shared cache builder sets it, so be
// mindful we don't collide.
static const uint32_t relativeMethodSelectorsAreDirectFlag = 0x40000000;

// class_data_bits_t is the class_t->data field (class_rw_t pointer plus flags)
// The extra bits are optimized for the retain/release and alloc/dealloc paths.

// Values for class_ro_t->flags
// These are emitted by the compiler and are part of the ABI.
// Note: See CGObjCNonFragileABIMac::BuildClassRoTInitializer in clang
// class is a metaclass
#define RO_META               (1<<0)
// class is a root class
#define RO_ROOT               (1<<1)
// class has .cxx_construct/destruct implementations
#define RO_HAS_CXX_STRUCTORS  (1<<2)
// class has +load implementation
// #define RO_HAS_LOAD_METHOD    (1<<3)
// class has visibility=hidden set
#define RO_HIDDEN             (1<<4)
// class has attribute(objc_exception): OBJC_EHTYPE_$_ThisClass is non-weak
#define RO_EXCEPTION          (1<<5)
// class has ro field for Swift metadata initializer callback
#define RO_HAS_SWIFT_INITIALIZER (1<<6)
// class compiled with ARC
#define RO_IS_ARC             (1<<7)
// class has .cxx_destruct but no .cxx_construct (with RO_HAS_CXX_STRUCTORS)
#define RO_HAS_CXX_DTOR_ONLY  (1<<8)
// class is not ARC but has ARC-style weak ivar layout
#define RO_HAS_WEAK_WITHOUT_ARC (1<<9)
// class does not allow associated objects on instances
#define RO_FORBIDS_ASSOCIATED_OBJECTS (1<<10)

// class is in an unloadable bundle - must never be set by compiler
#define RO_FROM_BUNDLE        (1<<29)
// class is unrealized future class - must never be set by compiler
#define RO_FUTURE             (1<<30)
// class is realized - must never be set by compiler
#define RO_REALIZED           (1<<31)

// Values for class_rw_t->flags
// These are not emitted by the compiler and are never used in class_ro_t.
// Their presence should be considered in future ABI versions.
// class_t->data is class_rw_t, not class_ro_t
#define RW_REALIZED           (1<<31)
// class is unresolved future class
#define RW_FUTURE             (1<<30)
// class is initialized
#define RW_INITIALIZED        (1<<29)
// class is initializing
#define RW_INITIALIZING       (1<<28)
// class_rw_t->ro is heap copy of class_ro_t
#define RW_COPIED_RO          (1<<27)
// class allocated but not yet registered
#define RW_CONSTRUCTING       (1<<26)
// class allocated and registered
#define RW_CONSTRUCTED        (1<<25)
// available for use; was RW_FINALIZE_ON_MAIN_THREAD
// #define RW_24 (1<<24)
// class +load has been called
#define RW_LOADED             (1<<23)
#if !SUPPORT_NONPOINTER_ISA
// class instances may have associative references
#define RW_INSTANCES_HAVE_ASSOCIATED_OBJECTS (1<<22)
#endif
// class has instance-specific GC layout
#define RW_HAS_INSTANCE_SPECIFIC_LAYOUT (1 << 21)
// class does not allow associated objects on its instances
#define RW_FORBIDS_ASSOCIATED_OBJECTS       (1<<20)
// class has started realizing but not yet completed it
#define RW_REALIZING          (1<<19)

#if CONFIG_USE_PREOPT_CACHES
// this class and its descendants can't have preopt caches with inlined sels
#define RW_NOPREOPT_SELS      (1<<2)
// this class and its descendants can't have preopt caches
#define RW_NOPREOPT_CACHE     (1<<1)
#endif

// class is a metaclass (copied from ro)
#define RW_META               RO_META // (1<<0)

// NOTE: MORE RW_ FLAGS DEFINED BELOW

// Values for class_rw_t->flags (RW_*), cache_t->_flags (FAST_CACHE_*),
// or class_t->bits (FAST_*).
//
// FAST_* and FAST_CACHE_* are stored on the class, reducing pointer indirection.

// class is a Swift class from the pre-stable Swift ABI
#define FAST_IS_SWIFT_LEGACY    (1UL<<0)
// class is a Swift class from the stable Swift ABI
#define FAST_IS_SWIFT_STABLE    (1UL<<1)
// class or superclass has default retain/release/autorelease/retainCount/
//   _tryRetain/_isDeallocating/retainWeakReference/allowsWeakReference
#define FAST_HAS_DEFAULT_RR     (1UL<<2)
// data pointer
#define FAST_DATA_MASK          0x00007ffffffffff8UL

// just the flags
#define FAST_FLAGS_MASK         0x0000000000000007UL
// this bit tells us *quickly* that it's a pointer to an rw, not an ro
#define FAST_IS_RW_POINTER      0x8000000000000000UL

// class or superclass has .cxx_construct implementation
#define RW_HAS_CXX_CTOR       (1<<18)
// class or superclass has .cxx_destruct implementation
#define RW_HAS_CXX_DTOR       (1<<17)
// class or superclass has default alloc/allocWithZone: implementation
// Note this is is stored in the metaclass.
#define RW_HAS_DEFAULT_AWZ    (1<<16)
// class's instances requires raw isa

#define RW_REQUIRES_RAW_ISA   (1<<15)
// class or superclass has default retain/release/autorelease/retainCount/
//   _tryRetain/_isDeallocating/retainWeakReference/allowsWeakReference
#define RW_HAS_DEFAULT_RR     (1<<14)
// class or superclass has default new/self/class/respondsToSelector/isKindOfClass
#define RW_HAS_DEFAULT_CORE   (1<<13)

// class is a Swift class from the pre-stable Swift ABI
#define FAST_IS_SWIFT_LEGACY  (1UL<<0)
// class is a Swift class from the stable Swift ABI
#define FAST_IS_SWIFT_STABLE  (1UL<<1)

namespace ObjectiveC
{
	class Ivar;
	class Property;

	class Method;

	class Category;
	class Protocol;

	class ObjCClass;

	class ObjCData;

	struct _objc_ivar
	{
		uint64_t offset;
		uint64_t name;
		uint64_t type;
		uint8_t size;
	};

	enum _objc_method_type
	{
		_objc_method_invalid_type = 0,
		_objc_method_instance_type,
		_objc_method_class_type
	};

	struct _objc_method
	{
		uint32_t name;
		uint32_t type;
		uint32_t offset;
	};

	struct _objc_protocol
	{
		char *name;
		uint64_t offset;
		struct _objc_method *method;
		uint32_t methodCount;
	};

	struct objc_category
	{
		char *category_name;
		char *class_name;

		struct _objc_method *instance_methods;
		struct _objc_method *class_methods;
		struct _objc_protocol *protocols;
	};

	struct _objc_class
	{
		struct _objc_class *superCls;
		char *className;
		struct _objc_ivar *ivar;
		uint32_t ivarCount;
		struct _objc_method *method;
		uint32_t methodCount;
		struct _objc_protocol *protocol;
		uint32_t protocolCount;
	};

	struct _objc_module
	{
		char *impName;
		struct _objc_class *symbol;
	};

	struct _objc_module_raw
	{
		uint32_t version;
		uint32_t size;
		uint32_t name;
		uint32_t symtab;
	};

	enum _objc_2_class_type
	{
		_objc_2_class_invalid_type = 0,
		_objc_2_class_class_type,
		_objc_2_class_metaclass_type
	};

	#define kObjc2SelRef	 "__objc_selrefs"
	#define kObjc2MsgRefs	"__objc_msgrefs"
	#define kObjc2ClassRefs "__objc_classrefs"
	#define kObjc2SuperRefs "__objc_superrefs"
	#define kObjc2ClassList "__objc_classlist"
	#define kObjc2NlClsList "__objc_nlclslist"
	#define kObjc2CatList	 "__objc_catlist"
	#define kObjc2NlCatList "__objc_nlcatlist"
	#define kObjc2ProtoList "__objc_protolist"
	#define kObjc2ProtoRefs "__objc_protorefs"

	struct _objc_2_class_method_info
	{
		uint32_t entrySize;
		uint32_t count;
	};

	struct _objc_2_class_protocol_info
	{
		uint64_t count;
	};

	struct _objc_2_class_ivar_info
	{
		uint32_t entrySize;
		uint32_t count;
	};

	struct _objc_2_class_property_info
	{
		uint32_t entrySize;
		uint32_t count;
	};

	struct _objc_2_class_method
	{
		uint32_t name;
		uint32_t type;
		int32_t imp;
	};

	struct _objc_2_method
	{
		mach_vm_address_t name;
		uint64_t type;
		mach_vm_address_t imp;
	};

	struct _objc_2_class_protocol
	{
		mach_vm_address_t isa;
		mach_vm_address_t name;

		mach_vm_address_t protocols;
		mach_vm_address_t instance_methods;
		mach_vm_address_t class_methods;
		mach_vm_address_t opt_instance_methods;
		mach_vm_address_t opt_class_methods;
		mach_vm_address_t instance_properties;

		uint32_t cb;
		uint32_t flags;
	};

	struct _objc_2_category
	{
		uint64_t category_name;
		uint64_t class_name;

		mach_vm_address_t instance_methods;
		mach_vm_address_t class_methods;
		mach_vm_address_t protocols;
		mach_vm_address_t properties;
	};

	struct _objc_2_class_ivar
	{
		mach_vm_address_t offset;
		mach_vm_address_t name;

		uint64_t type;
		uint32_t align;
		uint32_t size;
	};

	struct _objc_2_class_property
	{
		mach_vm_address_t name;

		uint64_t attributes;
	};

	struct _objc_2_class_data
	{
		uint32_t flags;
		uint32_t instanceStart;
		uint32_t instanceSize;
		uint32_t reserved;
		uint64_t iVarLayout;
		uint64_t name;

		mach_vm_address_t methods;
		mach_vm_address_t protocols;
		mach_vm_address_t ivars;

		mach_vm_address_t weakIVarLayout;
		mach_vm_address_t properties;
	};

	struct _objc_2_class
	{
		mach_vm_address_t isa;
		mach_vm_address_t superclass;
		mach_vm_address_t cache;
		mach_vm_address_t vtable;

		struct _objc_2_class_data *data;
	};

	struct _objc_2_class_rw_t 
	{
	    uint32_t flags;
	    uint16_t witness;

	    mach_vm_address_t ro_or_rw_ext;

	    mach_vm_address_t firstSubclass;
	    mach_vm_address_t nextSiblingClass;
	};

	struct _objc_2_class_rw_ext_t
	{
	    mach_vm_address_t ro;
	    
	    mach_vm_address_t methods;
	 	mach_vm_address_t properties;
	    mach_vm_address_t protocols;

	    mach_vm_address_t demangledName;

	    uint32_t version;
	};
};

typedef void* id;

namespace ObjectiveC
{
	mach_vm_address_t getClass(const char *name);

	const char*	   class_getName(mach_vm_address_t cls);
	mach_vm_address_t class_getSuperClass(mach_vm_address_t cls);
	mach_vm_address_t class_getSuperClass(mach_vm_address_t cls, mach_vm_address_t new_super);
	bool			  class_isMetaClass(mach_vm_address_t cls);
	mach_vm_address_t class_getInstanceVariable(mach_vm_address_t cls, const char *name);
	mach_vm_address_t class_getClassVariable(mach_vm_address_t cls, const char *name);
	bool			  class_addIvar(mach_vm_address_t cls, const char *name, size_t size, uint8_t alignment, const char *types);
	mach_vm_address_t class_getProperty(mach_vm_address_t cls, const char *name);
	bool			  class_addMethod(mach_vm_address_t cls, char *name, mach_vm_address_t mach_vm_address_t, const char *types);
	mach_vm_address_t class_getInstanceMethod(mach_vm_address_t cls, char *name);
	mach_vm_address_t class_getClassMethod(mach_vm_address_t cls, char *name);
	mach_vm_address_t class_getMethodImplementation(mach_vm_address_t cls, char * name);
	bool			  class_addProtocol(mach_vm_address_t cls, mach_vm_address_t protocol);
	bool			  class_addProperty(mach_vm_address_t cls, const char *name, mach_vm_address_t attributes, unsigned int attributeCount);

	mach_vm_address_t object_getClass(id obj);
	void			  object_setInstanceVariable(id obj, const char *name, void *value);
	mach_vm_address_t object_getInstanceVariable(id obj, const char *name);
	void			  object_setIvar(id obj, const char *name, id value);
	mach_vm_address_t object_getIvar(id obj, mach_vm_address_t ivar);
	const char *	  object_getClassName(id obj);
	mach_vm_address_t object_getClass(id obj);
	mach_vm_address_t object_setClass(id obj, mach_vm_address_t cls);

	char*			 method_getName(mach_vm_address_t m);
	mach_vm_address_t method_getImplementation(mach_vm_address_t m);
	mach_vm_address_t method_setImplementation(mach_vm_address_t m, mach_vm_address_t mach_vm_address_t);
	void			  method_exchangeImplementations(mach_vm_address_t m1, mach_vm_address_t m2);

	uint64_t 		  object_getDataBits(id obj);
	bool  			  object_isObjectiveC(id obj);
	bool 			  object_isSwift(id obj);

	class ObjC
	{
		public:
			virtual ~ObjC() = default;

			ObjCData* getMetadata() { return metadata; }
		protected:
			ObjCData *metadata;
	};

	ObjCData* parseObjectiveC(mrk::UserMachO *macho);

	std::Array<ObjCClass*>* parseClassList(ObjCData *data);
	std::Array<Category*>* parseCategoryList(ObjCData *data);
	std::Array<Protocol*>* parseProtocolList(ObjCData *data);

	enum MethodType
	{
		INSTANCE_METHOD = 0,
		CLASS_METHOD,
		OPT_INSTANCE_METHOD,
		OPT_CLASS_METHOD,
	};

	void parseMethodList(ObjCData *metadata, ObjC *object, std::Array<Method*> *methodList, enum MethodType methtype, struct _objc_2_class_method_info *methodInfo);

	void parsePropertyList(ObjCData *metadata, ObjC *object, std::Array<Property*> *propertyList, struct _objc_2_class_property_info *propertyInfo);

	mach_vm_address_t findSelectorsBase(mrk::UserMachO *macho);

	class Protocol : public ObjC
	{
		public:
			Protocol(ObjCData *data, struct _objc_2_class_protocol *prot);

			char* getName() { return name; }

			std::Array<Method*>* getInstanceMethods() { return &instance_methods; }
			std::Array<Method*>* getClassMethods() { return &class_methods; }

			std::Array<Method*>* getOptionalInstanceMethods() { return &optional_instance_methods; }
			std::Array<Method*>* getOptionalClassMethods() { return &optional_class_methods; }

			std::Array<Property*>* getInstanceProperties() { return &instance_properties; }

			mach_vm_address_t getOffset() { return offset; }

		private:
			struct _objc_2_class_protocol *protocol;

			char *name;

			mach_vm_address_t offset;

			std::Array<Method*> instance_methods;
			std::Array<Method*> class_methods;

			std::Array<Method*> optional_instance_methods;
			std::Array<Method*> optional_class_methods;

			std::Array<Property*> instance_properties;
	};

	class Category : public ObjC
	{
		public:
			Category(ObjCData *data, struct _objc_2_category *cat);

			char* getName() { return name; }

			char* getClassName() { return class_name; }

			std::Array<Method*>* getInstanceMethods() { return &instance_methods; }
			std::Array<Method*>* getClassMethods() { return &class_methods; }

			std::Array<Property*>* getProperties() { return &properties; }

		private:
			struct _objc_2_category *category;

			char *name;
			char *class_name;

			std::Array<Method*> class_methods;
			std::Array<Method*> instance_methods;

			std::Array<Property*> properties;
	};

	class Ivar
	{
		public:
			Ivar(ObjC *object, struct _objc_2_class_ivar *ivar);

			char* getName() { return name; }

			mach_vm_address_t getOffset() { return offset; }
			
			uint64_t getType() { return type; }

			size_t getSize() { return size; }

		private:
			ObjC *object;

			struct _objc_2_class_ivar *ivar;

			char *name;

			mach_vm_address_t offset;
			
			uint64_t type;
			
			size_t size;
	};

	class Property
	{
		public:
			Property(ObjC *object, struct _objc_2_class_property *property);

			char* getName() { return name; }

			char* getAttributes() { return attributes; }

		private:
			ObjC *object;

			struct _objc_2_class_property *property;

			char *name;

			char *attributes;
	};

	class Method
	{
		public:
			Method(ObjC *object, struct _objc_2_class_method *method);
			Method(ObjC *object, struct _objc_2_method *method);

			char* getName() { return name; }

			uint64_t getType() { return type; }

			mach_vm_address_t getImpl() { return impl; }

		private:
			ObjC *object;

			struct _objc_2_class_method *method;

			char *name;

			uint64_t type;

			mach_vm_address_t impl;
	};

	class ObjCClass : public ObjC
	{
		public:
			ObjCClass(ObjCData *metadata, struct _objc_2_class *c, bool metaclass);

			char* getName() { return name; }

			struct _objc_2_class* getClass() { return cls; }

			struct _objc_2_class_data* getData() { return data; }

			mrk::UserMachO* getMachO() { return macho; }

			ObjCClass* getSuperClass() { return super; }

			mach_vm_address_t getIsa() { return isa; }

			mach_vm_address_t getCache() { return cache; }

			mach_vm_address_t getVtable() { return vtable; }

			Method* getMethod(char *methodname);

			Protocol* getProtocol(char *protocolname);

			Ivar* getIvar(char *ivarname);

			Property* getProperty(char *propertyname);

			std::Array<Method*>* getMethods() { return &methods; }

			std::Array<Protocol*>* getProtocols() { return &protocols; }

			std::Array<Ivar*>* getIvars() { return &ivars; }

			std::Array<Property*>* getProperties() { return &properties; }

			bool isValid() { return (name && isa);}

			void parseMethods();

			void parseProtocols();

			void parseIvars();

			void parseProperties();

		private:
			mrk::UserMachO *macho;

			bool metaclass;

			struct _objc_2_class *cls;

			struct _objc_2_class_data *data;

			char *name;

			ObjCClass *super;

			mach_vm_address_t isa;
			mach_vm_address_t superclass;
			mach_vm_address_t cache;
			mach_vm_address_t vtable;

			std::Array<Method*> methods;

			std::Array<Protocol*> protocols;

			std::Array<Ivar*> ivars;

			std::Array<Property*> properties;
	};

	class ObjCData
	{
		public:
			ObjCData(mrk::UserMachO *macho) 
			{
				this->macho = macho;

				this->parseObjC();
			}

			mrk::UserMachO* getMachO() { return macho; }

			Segment* getDataSegment() { return data; }
			Segment* getDataConstSegment() { return data_const; }

			Section* getClassList() { return classlist; }
			Section* getCategoryList() { return catlist; }
			Section* getProtocolList() { return protolist; }

			Section* getSelRefs() { return selrefs; }
			Section* getProtoRefs() { return protorefs; }
			Section* getClassRefs() { return classrefs; }
			Section* getSuperRefs() { return superrefs; }

			Section* getIvars( ) { return ivar; }
			Section* getObjcData() { return objc_data; }

			void parseObjC();

			ObjCClass* getClassByName(char *classname);
			ObjCClass* getClassByIsa(mach_vm_address_t isa);

			Protocol* getProtocol(char *protoname);

			Category* getCategory(char *catname);

			Method* getMethod(char *classname, char *methodname);

			Ivar* getIvar(char *classname, char *ivarname);

			Property* getProperty(char *classname, char *propertyname);

		private:
			mrk::UserMachO *macho;

			std::Array<ObjCClass*> *classes;
			std::Array<Category*> *categories;
			std::Array<Protocol*> *protocols;

			Segment *data;
			Segment *data_const;

			Section *classlist;
			Section *catlist;
			Section *protolist;

			Section *selrefs;
			Section *protorefs;
			Section *classrefs;
			Section *superrefs;

			Section *ivar;
			Section *objc_data;
	};
};


#endif