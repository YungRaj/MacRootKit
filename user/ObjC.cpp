#include "ObjC.hpp"

#include "MachO.hpp"
#include "UserMachO.hpp"

#include "PAC.hpp"

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
			ObjCClass *metac;

			uint64_t cls = reinterpret_cast<uint64_t>(ptrauth_strip(*(clslist + off / sizeof(uint64_t)), ptrauth_key_process_dependent_data));

			struct _objc_2_class *objc_class = reinterpret_cast<struct _objc_2_class*>(cls);

			if(objc_class)
			{
				c = new ObjCClass(data, objc_class, false);

				classes->add(c);
			}

			struct _objc_2_class *objc_metaclass = reinterpret_cast<struct _objc_2_class*>(objc_class->isa);

			if(objc_metaclass)
			{
				metac = new ObjCClass(data, objc_class, true);

				classes->add(metac);
			}
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

			uint64_t category = reinterpret_cast<uint64_t>(ptrauth_strip(*(categorylist + off / sizeof(uint64_t)), ptrauth_key_process_dependent_data));

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

			uint64_t proto = reinterpret_cast<uint64_t>(ptrauth_strip(*(protolist + off / sizeof(uint64_t)), ptrauth_key_process_dependent_data));

			struct _objc_protocol *objc_protocol = reinterpret_cast<struct _objc_protocol*>(proto);

			p = new Protocol(data, objc_protocol);

			protocols->add(p);
		}

		return protocols;
	}
}

namespace ObjectiveC
{
	ObjCClass::ObjCClass(ObjCData *metadata, struct _objc_2_class *c, bool metaclass)
	{
		this->metadata = metadata;
		this->metaclass = metaclass;
		this->cls = c;
		this->super = NULL;

		this->data = reinterpret_cast<struct _objc_2_class_data*>(data);

		this->name = reinterpret_cast<char*>(data->name);

		this->isa = reinterpret_cast<mach_vm_address_t>(c->isa);
		this->superclass = reinterpret_cast<mach_vm_address_t>(c->superclass);
		this->cache = reinterpret_cast<mach_vm_address_t>(c->cache);
		this->vtable = reinterpret_cast<mach_vm_address_t>(c->vtable);

		this->parseMethods();
		this->parseIvars();
		this->parseProperties();
	}

	Ivar::Ivar(ObjCClass *cls, struct _objc_2_class_ivar *ivar)
	{
		this->cls = cls;
		this->ivar = ivar;
		this->name = reinterpret_cast<char*>(ptrauth_strip(ivar->name, ptrauth_key_process_dependent_data));
		this->type = ivar->type;
		this->size = ivar->size;
	}

	Property::Property(ObjCClass *cls, struct _objc_2_class_property *property)
	{
		this->cls = cls;
		this->property = property;
		this->name = reinterpret_cast<char*>(ptrauth_strip(property->name, ptrauth_key_process_dependent_data));
		this->attributes = reinterpret_cast<char*>(ptrauth_strip(property->attributes, ptrauth_key_process_dependent_data));
	}

	Method::Method(ObjCClass *cls, struct _objc_method *method)
	{
		mach_vm_address_t name_;

		name_ = *reinterpret_cast<mach_vm_address_t*>(method->name + (mach_vm_address_t) &method->name);
		
		this->cls = cls;
		this->method = method;
		this->name = reinterpret_cast<char*>(ptrauth_strip(name_, ptrauth_key_process_dependent_data));
		this->impl = reinterpret_cast<mach_vm_address_t>(method->offset + (mach_vm_address_t) &method->offset - cls->getMetadata()->getMachO()->getBase());
		this->type = method->type;
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

	void ObjCClass::parseMethods()
	{
		struct _objc_2_class *c;

		struct _objc_2_class_data *d;

		struct _objc_2_class_method_info *methods;

		off_t off;

		c = this->cls;

		d = this->data;

		methods = reinterpret_cast<struct _objc_2_class_method_info*>(d->methods);

		off = sizeof(struct _objc_2_class_method_info);

		MAC_RK_LOG("\t\t\tMethods\n");

		for(int i = 0; i < methods->count; i++)
		{
			struct _objc_method *method = reinterpret_cast<struct _objc_method*>(reinterpret_cast<uint8_t*>(methods) + off);

			Method *meth = new Method(this, method);

			this->methods.add(meth);

			if(metaclass)
			{
				MAC_RK_LOG("\t\t\t\t0x%08llx: +%s\n", meth->getImpl(), meth->getName());
			} else
			{
				MAC_RK_LOG("\t\t\t\t0x%08llx: -%s\n", meth->getImpl(), meth->getName());
			}
		
			off += sizeof(struct _objc_method);
		}
	}

	void ObjCClass::parseProtocols()
	{
		struct _objc_2_class *c;

		struct _objc_2_class_data *d;

		struct _objc_2_class_protocol_info *protocols;

		off_t off;

		c = this->cls;

		d = this->data;

		protocols = reinterpret_cast<struct _objc_2_class_protocol_info*>(data->protocols);

		off = sizeof(struct _objc_2_class_protocol_info);
	}

	void ObjCClass::parseIvars()
	{
		struct _objc_2_class *c;

		struct _objc_2_class_data *d;

		struct _objc_2_class_ivar_info *ivars;

		off_t off;

		c = this->cls;

		d = this->data;

		ivars = reinterpret_cast<struct _objc_2_class_ivar_info*>(d->ivars);

		off = sizeof(struct _objc_2_class_ivar_info);

		MAC_RK_LOG("\t\t\tIvars\n");

		for(int i = 0; i < ivars->count; i++)
		{
			struct _objc_2_class_ivar *ivar = reinterpret_cast<struct _objc_2_class_ivar*>(reinterpret_cast<uint8_t*>(ivars) + off);

			Ivar *iv = new Ivar(this, ivar);

			this->ivars.add(iv);

			MAC_RK_LOG("\t\t\t\t0x%08llx: %s\n", iv->getOffset(), iv->getName());

			off += sizeof(struct _objc_ivar);
		}
	}

	void ObjCClass::parseProperties()
	{
		struct _objc_2_class *c;

		struct _objc_2_class_data *d;

		struct _objc_2_class_property_info *properties;

		off_t off;

		c = this->cls;

		d = this->data;

		properties = reinterpret_cast<struct _objc_2_class_property_info*>(d->properties);

		MAC_RK_LOG("\t\t\tProperties\n");

		for(int i = 0; i < properties->count; i++)
		{
			struct _objc_2_class_property *property = reinterpret_cast<struct _objc_2_class_property*>(reinterpret_cast<uint8_t*>(properties) + off);

			Property *prop = new Property(this, property);

			this->properties.add(prop);

			MAC_RK_LOG("\t\t\t\t%s %s\n", prop->getAttributes(), prop->getName());

			off += sizeof(struct _objc_2_class_property);
		}
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