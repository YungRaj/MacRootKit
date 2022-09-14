#ifndef __OBJC_H_
#define __OBJC_H_

#include <mach/mach_types.h>

#include "objc.h"

#include "Array.hpp"

class MachO;
class UserMachO;

class Segment;
class Section;

namespace ObjectiveC
{
	class Ivar;
	class Property;

	class Method;

	class Category;
	class Protocol;

	class ObjCClass;

	class ObjCData;
};

typedef void* id;

namespace ObjectiveC
{
	mach_vm_address_t getClass(const char *name);

	const char*       class_getName(mach_vm_address_t cls);
	mach_vm_address_t class_getSuperClass(mach_vm_address_t cls);
	mach_vm_address_t class_getSuperClass(mach_vm_address_t cls, mach_vm_address_t new_super);
	bool              class_isMetaClass(mach_vm_address_t cls);
	mach_vm_address_t class_getInstanceVariable(mach_vm_address_t cls, const char *name);
	mach_vm_address_t class_getClassVariable(mach_vm_address_t cls, const char *name);
	bool              class_addIvar(mach_vm_address_t cls, const char *name, size_t size, uint8_t alignment, const char *types);
	mach_vm_address_t class_getProperty(mach_vm_address_t cls, const char *name);
	bool              class_addMethod(mach_vm_address_t cls, char *name, mach_vm_address_t mach_vm_address_t, const char *types);
	mach_vm_address_t class_getInstanceMethod(mach_vm_address_t cls, char *name);
	mach_vm_address_t class_getClassMethod(mach_vm_address_t cls, char *name);
	mach_vm_address_t               class_getMethodmach_vm_address_tlementation(mach_vm_address_t cls, char * name);
	bool              class_addProtocol(mach_vm_address_t cls, mach_vm_address_t protocol);
	bool              class_addProperty(mach_vm_address_t cls, const char *name, mach_vm_address_t attributes, unsigned int attributeCount);

	mach_vm_address_t object_getClass(id obj);
	void              object_setInstanceVariable(id obj, const char *name, void *value);
	mach_vm_address_t object_getInstanceVariable(id obj, const char *name);
	void              object_setIvar(id obj, const char *name, id value);
	mach_vm_address_t object_getIvar(id obj, mach_vm_address_t ivar);
	const char *      object_getClassName(id obj);
	mach_vm_address_t object_getClass(id obj);
	mach_vm_address_t object_setClass(id obj, mach_vm_address_t cls);

	char*             method_getName(mach_vm_address_t m);
	mach_vm_address_t method_getmach_vm_address_tlementation(mach_vm_address_t m);
	mach_vm_address_t method_setmach_vm_address_tlementation(mach_vm_address_t m, mach_vm_address_t mach_vm_address_t);
	void              method_exchangeImplementations(mach_vm_address_t m1, mach_vm_address_t m2);

	ObjCData* parseObjectiveC(UserMachO *macho);

	Array<ObjCClass*>* parseClassList(ObjCData *data);
	Array<Category*>* parseCategoryList(ObjCData *data);
	Array<Protocol*>* parseProtocolList(ObjCData *data);

	class Protocol
	{
		public:
			Protocol(struct _objc_protocol *prot)
			{

			}

			char* getName() { return name; }

			mach_vm_address_t getOffset() { return offset; }

		private:
			char *name;

			mach_vm_address_t offset;

			Array<Method*> methods;
	};

	class Category
	{
		public:
			Category(struct _objc_category *cat)
			{

			}

			char* getName() { return category_name; }

			char* getClassName() { return class_name; }

		private:
			char *category_name;
			char *class_name;

			Array<Method*> class_methods;
			Array<Method*> instance_methods;
	};

	class Ivar
	{
		public:
			Ivar(struct _objc_2_class_ivar *ivar)
			{

			}

			char* getName() { return name; }

			mach_vm_address_t getOffset() { return offset; }
			
			uint64_t getType() { return type; }

			size_t getSize() { return size; }

		private:
			char *name;

			mach_vm_address_t offset;
			
			uint64_t type;
			
			size_t size;
	};

	class Property
	{
		public:
			Property(struct _objc_2_class_property *property)
			{

			}

			char* getName() { return name; }

			uint64_t getAttributes() { return attributes; }

		private:
			char *name;

			uint64_t attributes;
	};

	class Method
	{
		public:
			Method(struct _objc2_class_method *method)
			{

			}

			char* getName() { return name; }

			uint64_t getType() { return type; }

			mach_vm_address_t getmach_vm_address_tl() { return mach_vm_address_tl; }

		private:
			char *name;

			uint64_t type;

			mach_vm_address_t mach_vm_address_tl;
	};

	class ObjCClass
	{
		public:
			ObjCClass(struct _objc_2_class *c)
			{

			}

			char* getName() { return name; }

			ObjCClass* getSuperClass() { return superclass; }

			mach_vm_address_t getIsa() { return isa; }

			mach_vm_address_t getCache() { return cache; }

			mach_vm_address_t getVtable() { return vtable; }

			Method* getMethod(char *methodname);

			Protocol* getProtocol(char *protocolname);

			Ivar* getIvar(char *ivarname);

			Property* getProperty(char *propertyname);

			Array<Method*>* getMethods() { return &methods; }

			Array<Protocol*>* getProtocols() { return &protocols; }

			Array<Ivar*>* getIvars() { return &ivars; }

			Array<Property*>* getProperties() { return &properties; }

		private:
			char *name;

			ObjCClass *superclass;

			mach_vm_address_t isa;
			mach_vm_address_t cache;
			mach_vm_address_t vtable;

			Array<Method*> methods;

			Array<Protocol*> protocols;

			Array<Ivar*> ivars;

			Array<Property*> properties;
	};

	class ObjCData
	{
		public:
			ObjCData(MachO *macho) 
			{
				this->macho = macho;

				this->parseObjC();
			}

			MachO* getMachO() { return macho; }

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

			Protocol* getProtocol(char *protoname);

			Category* getCategory(char *catname);

			Method* getMethod(char *classname, char *methodname);

			Ivar* getIvar(char *classname, char *ivarname);

			Property* getProperty(char *classname, char *propertyname);

		private:
			MachO *macho;

			Array<ObjCClass*> *classes;
			Array<Category*> *categories;
			Array<Protocol*> *protocols;

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