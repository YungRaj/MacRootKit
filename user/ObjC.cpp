#include "ObjC.hpp"

#include "MachO.hpp"
#include "UserMachO.hpp"

#include <assert.h>
#include <string.h>

namespace ObjectiveC
{
	ObjCData* parseObjectiveC(UserMachO *macho)
	{
		return new ObjCData(macho);
	}

	Array<ObjCClass*>* parseClassList(ObjCData *data)
	{
		Array<ObjCClass*> *classes = new Array<ObjCClass*>();

		Section *classlist = data->getClassList();

		Section *classrefs = data->getClassRefs();
		Section *superrefs = data->getSuperRefs();

		Section *ivars = data->getIvars();
		Section *objc_data = data->getObjcData();
		
		size_t classlist_size = classlist->getSize();

		mach_vm_address_t address = classlist->getAddress();

		uint64_t *clslist = reinterpret_cast<uint64_t*>(address);

		for(uint64_t off = 0; off < classlist_size; off = off + sizeof(uint64_t))
		{
			ObjCClass *c;

			uint64_t cls = *(clslist + off / sizeof(uint64_t));

			struct _objc_2_class *objc_class = reinterpret_cast<struct _objc_2_class*>(cls);

			c = new ObjCClass(objc_class);

			classes->add(c);
		}

		return classes;
	}

	Array<Category*>* parseCategoryList(ObjCData *data)
	{
		Array<Category*> *categories = new Array<Category*>();

		Section *catlist = data->getCategoryList();

		size_t catlist_size = catlist->getSize();

		mach_vm_address_t address = catlist->getAddress();

		uint64_t *categorylist = reinterpret_cast<uint64_t*>(address);

		for(uint64_t off = 0; off < catlist_size; off = off + sizeof(uint64_t))
		{
			Category *cat;

			uint64_t category = *(categorylist + off / sizeof(uint64_t));

			struct _objc_category *objc_category = reinterpret_cast<struct _objc_category*>(category);

			cat = new Category(objc_category);

			categories->add(cat);
		}

		return categories;
	}

	Array<Protocol*>* parseProtocolList(ObjCData *data)
	{
		Array<Protocol*> *protocols = new Array<Protocol*>();

		Section *protlist = data->getProtocolList();

		Section *protrefs = data->getProtoRefs();

		size_t protolist_size = protlist->getSize();

		mach_vm_address_t address = protlist->getAddress();

		uint64_t *protolist = reinterpret_cast<uint64_t*>(address);

		for(uint64_t off = 0; off < protolist_size; off = off + sizeof(uint64_t))
		{
			Protocol *p;

			uint64_t proto = *(protolist + off / sizeof(uint64_t));

			struct _objc_protocol *objc_protocol = reinterpret_cast<struct _objc_protocol*>(proto);

			p = new Protocol(objc_protocol);

			protocols->add(p);
		}

		return protocols;
	}
}

namespace ObjectiveC
{
	Protocol* ObjCClass::getProtocol(char *protocolname)
	{
		for(int i = 0; i < this->getProtocols()->getSize(); i++)
		{
			Protocol *protocol = this->getProtocols()->get(i);

			if(strcmp(protocol->getName(), protocolname) == 0)
			{
				return protocol;
			}
		}

		return NULL;
	}

	Method* ObjCClass::getMethod(char *methodname)
	{
		for(int i = 0; i < this->getMethods()->getSize(); i++)
		{
			Method *method = this->getMethods()->get(i);

			if(strcmp(method->getName(), methodname) == 0)
			{
				return method;
			}
		}

		return NULL;
	}

	Ivar* ObjCClass::getIvar(char *ivarname)
	{
		for(int i = 0; i < this->getIvars()->getSize(); i++)
		{
			Ivar *ivar = this->getIvars()->get(i);

			if(strcmp(ivar->getName(), ivarname) == 0)
			{
				return ivar;
			}
		}

		return NULL;
	}

	Property* ObjCClass::getProperty(char *propertyname)
	{
		for(int i = 0; i < this->getProperties()->getSize(); i++)
		{
			Property *property = this->getProperties()->get(i);

			if(strcmp(property->getName(), propertyname) == 0)
			{
				return property;
			}
		}

		return NULL;
	}
}

namespace ObjectiveC
{
	void ObjCData::parseObjC()
	{
		this->data = this->macho->getSegment("__DATA");
		this->data_const = this->macho->getSegment("__DATA_CONST");

		this->classlist = this->macho->getSection("__DATA_CONST", "__objc_classlist");
		this->catlist = this->macho->getSection("__DATA_CONST", "__objc_catlist");
		this->protolist = this->macho->getSection("__DATA_CONST", "__objc_protolist");

		this->selrefs = this->macho->getSection("__DATA", "__objc_selrefs");
		this->protorefs = this->macho->getSection("__DATA", "__objc_protorefs");
		this->classrefs = this->macho->getSection("__DATA", "__objc_classrefs");
		this->superrefs = this->macho->getSection("__DATA", "__objc_superrefs");

		this->ivar = this->macho->getSection("__DATA", "__objc_ivar");
		this->objc_data = this->macho->getSection("__DATA", "__objc_data");

		assert(this->data);
		assert(this->data);

		assert(this->classlist);
		assert(this->catlist);
		assert(this->protolist);

		assert(this->selrefs);
		assert(this->protorefs);
		assert(this->classrefs);
		assert(this->superrefs);

		assert(this->ivar);
		assert(this->objc_data);

		this->classes = parseClassList(this);
		this->categories = parseCategoryList(this);
		this->protocols = parseProtocolList(this);
	}

	ObjCClass* ObjCData::getClassByName(char *classname)
	{
		for(int i = 0; i < this->classes->getSize(); i++)
		{
			ObjCClass *cls = this->classes->get(i);

			if(strcmp(cls->getName(), classname) == 0)
			{
				return cls;
			}
		}

		return NULL;
	}

	Protocol* ObjCData::getProtocol(char *protoname)
	{
		for(int i = 0; i < this->protocols->getSize(); i++)
		{
			Protocol *protocol = this->protocols->get(i);

			if(strcmp(protocol->getName(), protoname) == 0)
			{
				return protocol;
			}
		}

		return NULL;
	}

	Category* ObjCData::getCategory(char *catname)
	{
		for(int i = 0; i < this->categories->getSize(); i++)
		{
			Category *category = this->categories->get(i);

			if(strcmp(category->getName(), catname) == 0)
			{
				return category;
			}
		}

		return NULL;
	}

	Method* ObjCData::getMethod(char *classname, char *methodname)
	{
		ObjCClass *cls = this->getClassByName(classname);

		return cls ? cls->getMethod(methodname) : NULL;
	}

	Ivar* ObjCData::getIvar(char *classname, char *ivarname)
	{
		ObjCClass *cls = this->getClassByName(classname);

		return cls ? cls->getIvar(ivarname) : NULL;
	}

	Property* ObjCData::getProperty(char *classname, char *propertyname)
	{
		ObjCClass *cls = this->getClassByName(classname);

		return cls ? cls->getProperty(propertyname) : NULL;
	}
}