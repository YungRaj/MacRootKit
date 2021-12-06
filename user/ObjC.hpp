#ifndef __OBJC_H_
#define __OBJC_H_

#include <mach/mach_types.h>

#include "objc.h"

#include "Array.hpp"

class MachO;
class Segment;
class Section;

class Ivar;
class Property;

class Method;
class Protocol;

class ObjCClass;

class ObjC;

namespace ObjectiveC
{
	class Protocol
	{
	public:
		Protocol(struct _objc_2_class_property *property);

		char* getName() { return name; }

		mach_vm_address_t getOffset() { return offset; }

	private:
		char *name;

		mach_vm_address_t offset;

		Array<Method*> methods;
	};

	class Ivar
	{
	public:
		Ivar(struct _objc_2_class_ivar *ivar);

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
		Property(struct _objc_2_class_property *property);

		char* getName() { return name; }

		uint64_t getAttributes() { return attributes; }

	private:
		char *name;

		uint64_t attributes;
	};

	class Method
	{
	public:
		Method(struct _objc2_class_method *method);

		char* getName() { return name; }

		uint64_t getType() { return type; }

		mach_vm_address_t getImpl() { return impl; }

	private:
		char *name;

		uint64_t type;

		mach_vm_address_t impl;
	};

	class ObjCClass
	{
	public:
		ObjCClass(struct _objc_2_class *c);

		ObjCClass* getSuperClass() { return superclass; }

		mach_vm_address_t getIsa() { return isa; }

		mach_vm_address_t getCache() { return cache; }

		mach_vm_address_t getVtable() { return vtable; }

		Method* getMethod(char *name);

		Protocol* getProtocol(char *name);

		Ivar* getIvar(char *name);

		Property* getProperty(char *name);

	private:
		char *name;

		ObjCClass *superclass;

		mach_vm_address_t isa;
		mach_vm_address_t cache;
		mach_vm_address_t vtable;

		Array<Method*> methods;

		Array<Protocol*> protocol;

		Array<Ivar*> ivars;

		Array<Property*> properties;

	};

	class ObjCData
	{
	public:
		ObjCData(MachO *macho, Segment *objc, Section *classlist) 
		{
			this->macho = macho;
			this->objc = objc;
			this->classlist = classlist;

			this->parseObjC();
		}

		MachO* getMachO() { return macho; }

		void parseObjC();

		ObjCClass* getClassByName(char *classname);

		Protocol* getProtocol(char *classname, char *protocol);

		Method* getMethod(char *classname, char *method);

		Ivar* getIvar(char *classname, char *ivar);

		Property getProperty(char *classname, char *property);


	private:
		MachO *macho;

		Array<ObjCClass*> classes;

		Segment *objc;
		Section *classlist;
	};
};


#endif