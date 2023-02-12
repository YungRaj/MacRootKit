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

#define RELATIVE_METHODS_SELECTORS_ARE_DIRECT_FLAG = 0x40000000
#define RELATIVE_METHOD_FLAG = 0x80000000
#define METHOD_LIST_FLAGS_MASK = 0xFFFF0000

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
		uint64_t name;
		uint64_t type;
		uint64_t imp;
	};

	struct _objc_2_class_protocol
	{
		uint64_t isa;
		uint64_t name;
		uint64_t protocols;
		uint64_t instance_methods;
		uint64_t class_methods;
		uint64_t opt_instance_methods;
		uint64_t opt_class_methods;
		uint64_t instance_properties;
		uint32_t cb;
		uint32_t flags;
	};

	struct _objc_2_category
	{
		uint64_t category_name;
		uint64_t class_name;

		uint64_t instance_methods;
		uint64_t class_methods;
		uint64_t protocols;
		uint64_t properties;
	};

	struct _objc_2_class_ivar
	{
		uint64_t offset;
		uint64_t name;
		uint64_t type;
		uint32_t align;
		uint32_t size;
	};

	struct _objc_2_class_property
	{
		uint64_t name;
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
		//char*
		uint64_t methods;
		//struct _objc_2_class_method_info*
		uint64_t protocols;
		//struct _objc_2_class_protocol_info*
		uint64_t ivars;
		//struct _objc_2_class_ivar_info*
		uint64_t weakIVarLayout;
		uint64_t properties; //struct _objc_2_class_property_info*
	};

	struct _objc_2_class
	{
		uint64_t isa;
		uint64_t superclass;
		uint64_t cache;
		uint64_t vtable;
		struct _objc_2_class_data *data;
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