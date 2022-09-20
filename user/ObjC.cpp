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
		UserMachO *macho = data->getMachO();

		Array<ObjCClass*> *classes = new Array<ObjCClass*>();

		Section *classlist = data->getClassList();

		Section *classrefs = data->getClassRefs();
		Section *superrefs = data->getSuperRefs();

		Section *ivars = data->getIvars();
		Section *objc_data = data->getObjcData();
		
		size_t classlist_size = classlist->getSize();

		mach_vm_address_t address = classlist->getAddress();

		mach_vm_address_t base = macho->getBase() - data->getMachO()->getAslrSlide();

		mach_vm_address_t header = reinterpret_cast<mach_vm_address_t>(macho->getMachHeader());

		uint64_t *clslist = reinterpret_cast<uint64_t*>(header + classlist->getOffset());

		for(uint64_t off = 0; off < classlist_size; off = off + sizeof(uint64_t))
		{
			ObjCClass *c;
			ObjCClass *metac;

			uint64_t cls = macho->getBufferAddress(*(clslist + off / sizeof(uint64_t)) & 0xFFFFFFFFFFF);

			if(!cls)
			{
				cls = header + *(clslist + off / sizeof(uint64_t)) & 0xFFFFFFFFFFF;
			}

			struct _objc_2_class *objc_class = reinterpret_cast<struct _objc_2_class*>(cls);

			if(objc_class)
			{
				c = new ObjCClass(data, objc_class, false);

				classes->add(c);
			}

			/*
			struct _objc_2_class *objc_metaclass = reinterpret_cast<struct _objc_2_class*>(objc_class->isa);

			if(objc_metaclass)
			{
				metac = new ObjCClass(data, objc_class, true);

				classes->add(metac);
			}*/
		}

		return classes;
	}

	Array<Category*>* parseCategoryList(ObjCData *data)
	{
		Array<Category*> *categories = new Array<Category*>();

		/*
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
		*/

		return categories;
	}

	Array<Protocol*>* parseProtocolList(ObjCData *data)
	{
		Array<Protocol*> *protocols = new Array<Protocol*>();

		/*
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
		*/

		return protocols;
	}
}

namespace ObjectiveC
{
	ObjCClass::ObjCClass(ObjCData *metadata, struct _objc_2_class *c, bool metaclass)
	{
		this->macho = metadata->getMachO();
		this->metadata = metadata;
		this->metaclass = metaclass;
		this->cls = c;
		this->super = NULL;

		// printf("data = 0x%llx \n", (uint64_t) c->data);

		this->data = reinterpret_cast<struct _objc_2_class_data*>((uint64_t) c->data & 0xFFFFFFF8);

		// printf("data = 0x%llx \n", data);

		if(this->macho->sectionForOffset(reinterpret_cast<mach_vm_address_t>(this->data)))
		{
			this->data = reinterpret_cast<struct _objc_2_class_data*>((uint64_t) this->data + reinterpret_cast<mach_vm_address_t>(this->macho->getMachHeader()));
			this->name = reinterpret_cast<char*>((data->name & 0xFFFFFFFFFF) + reinterpret_cast<mach_vm_address_t>(this->macho->getMachHeader()));

			if(metaclass)
				printf("\t\t$OBJC_METACLASS_%s\n",name);
			else
				printf("\t\t$OBJC_CLASS_%s\n",name);

			this->isa = reinterpret_cast<mach_vm_address_t>(c->isa);
			this->superclass = reinterpret_cast<mach_vm_address_t>(c->superclass);
			this->cache = reinterpret_cast<mach_vm_address_t>(c->cache);
			this->vtable = reinterpret_cast<mach_vm_address_t>(c->vtable);

			this->parseMethods();
			this->parseIvars();
			this->parseProperties();
		} else if(this->macho->sectionForAddress(reinterpret_cast<mach_vm_address_t>(this->data)))
		{
			this->name = reinterpret_cast<char*>(data->name);

			this->isa = reinterpret_cast<mach_vm_address_t>(c->isa);
			this->superclass = reinterpret_cast<mach_vm_address_t>(c->superclass);
			this->cache = reinterpret_cast<mach_vm_address_t>(c->cache);
			this->vtable = reinterpret_cast<mach_vm_address_t>(c->vtable);

			this->parseMethods();
			this->parseIvars();
			this->parseProperties();
		}
	}

	Ivar::Ivar(ObjCClass *cls, struct _objc_2_class_ivar *ivar)
	{
		this->cls = cls;
		this->ivar = ivar;
		this->offset = reinterpret_cast<mach_vm_address_t>((ivar->offset & 0xFFFFFF) + reinterpret_cast<mach_vm_address_t>(cls->getMachO()->getMachHeader()));
		this->name = reinterpret_cast<char*>((ivar->name & 0xFFFFFF) + reinterpret_cast<char*>(cls->getMachO()->getMachHeader()));
		this->type = ivar->type;
		this->size = ivar->size;
	}

	Property::Property(ObjCClass *cls, struct _objc_2_class_property *property)
	{
		this->cls = cls;
		this->property = property;
		this->name = reinterpret_cast<char*>((property->name & 0xFFFFFF) + reinterpret_cast<char*>(cls->getMachO()->getMachHeader()));
		this->attributes = reinterpret_cast<char*>((property->attributes & 0xFFFFFF) + reinterpret_cast<char*>(cls->getMachO()->getMachHeader()));
	}

	Method::Method(ObjCClass *cls, struct _objc_2_class_method *method)
	{
		mach_vm_address_t pointer_to_name;

		pointer_to_name = reinterpret_cast<mach_vm_address_t>(&method->name) + reinterpret_cast<uint64_t>(method->name & 0xFFFFFF);

		// printf("method->name = 0x%llx pointer_to_name = 0x%llx &method->name = 0x%llx\n", method->name, pointer_to_name, (uint64_t) &method->name);

		pointer_to_name = reinterpret_cast<mach_vm_address_t>(cls->getMachO()->getMachHeader()) + (*(uint64_t*) pointer_to_name & 0xFFFFFF);
		
		// printf("method = 0x%llx name = 0x%llx\n", method->name, pointer_to_name);

		this->cls = cls;
		this->method = method;
		this->name = reinterpret_cast<char*>(pointer_to_name);
		this->impl = reinterpret_cast<mach_vm_address_t>((method->imp & 0xFFFFFF) + reinterpret_cast<mach_vm_address_t>(cls->getMachO()->getMachHeader()));

		// printf("@ impl = 0x%llx\n", *(uint32_t*) this->impl);

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

		if(!d->methods)
		{
			return;
		}

		methods = reinterpret_cast<struct _objc_2_class_method_info*>((d->methods & 0xFFFFFFFF) + reinterpret_cast<mach_vm_address_t>(this->macho->getMachHeader()));

		off = sizeof(struct _objc_2_class_method_info);

		MAC_RK_LOG("\t\t\tMethods\n");

		for(int i = 0; i < methods->count; i++)
		{
			struct _objc_2_class_method *method = reinterpret_cast<struct _objc_2_class_method*>(reinterpret_cast<uint8_t*>(methods) + off);

			// printf("method = 0x%llx\n", method);

			Method *meth = new Method(this, method);

			this->methods.add(meth);

			if(metaclass)
			{
				MAC_RK_LOG("\t\t\t\t0x%08llx: +%s\n", meth->getImpl(), meth->getName());
			} else
			{
				MAC_RK_LOG("\t\t\t\t0x%08llx: -%s\n", meth->getImpl(), meth->getName());
			}
		
			off += sizeof(struct _objc_2_class_method);
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

		if(!d->ivars)
		{
			return;
		}

		ivars = reinterpret_cast<struct _objc_2_class_ivar_info*>((d->ivars & 0xFFFFFFFF) + reinterpret_cast<mach_vm_address_t>(this->macho->getMachHeader()));

		off = sizeof(struct _objc_2_class_ivar_info);

		MAC_RK_LOG("\t\t\tIvars\n");
		
		for(int i = 0; i < ivars->count; i++)
		{
			struct _objc_2_class_ivar *ivar = reinterpret_cast<struct _objc_2_class_ivar*>(reinterpret_cast<uint8_t*>(ivars) + off);

			Ivar *iv = new Ivar(this, ivar);

			this->ivars.add(iv);

			MAC_RK_LOG("\t\t\t\t0x%08llx: %s\n", iv->getOffset(), iv->getName());

			off += sizeof(struct _objc_2_class_ivar);
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

		if(!d->properties)
		{
			return;
		}

		properties = reinterpret_cast<struct _objc_2_class_property_info*>((d->properties & 0xFFFFFFFF) + reinterpret_cast<mach_vm_address_t>(this->macho->getMachHeader()));

		off = sizeof(struct _objc_2_class_property_info);

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
		MAC_RK_LOG("We got here!\n");

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