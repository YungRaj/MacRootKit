#include "ObjC.hpp"

#include "MachO.hpp"
#include "UserMachO.hpp"

#include "PAC.hpp"

#include <assert.h>
#include <string.h>

namespace ObjectiveC
{

ObjCData* parseObjectiveC(mrk::UserMachO *macho)
{
	return new ObjCData(macho);
}

std::Array<ObjCClass*>* parseClassList(ObjCData *data)
{
	mrk::UserMachO *macho = data->getMachO();

	std::Array<ObjCClass*> *classes = new std::Array<ObjCClass*>();

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

		uint64_t cls = macho->getBufferAddress(((*(clslist + off / sizeof(uint64_t))) & 0xFFFFFFFFFFF) - macho->getAslrSlide());

		if(!cls)
		{
			cls = header + *(clslist + off / sizeof(uint64_t)) & 0xFFFFFFFFFFF;
		}

		struct _objc_2_class *objc_class = reinterpret_cast<struct _objc_2_class*>(cls);

		if(objc_class)
		{
			c = new ObjCClass(data, objc_class, false);

			if(c->isValid())
			{
				classes->add(c);
			}
		}

		struct _objc_2_class *objc_metaclass = reinterpret_cast<struct _objc_2_class*>((objc_class->isa & 0xFFFFFFFF) + header);

		if(objc_metaclass && objc_class != objc_metaclass)
		{
			metac = new ObjCClass(data, objc_metaclass, true);

			if(metac->isValid())
			{
				classes->add(metac);
			}
		}
	}

	return classes;
}

std::Array<Category*>* parseCategoryList(ObjCData *data)
{
	mrk::UserMachO *macho = data->getMachO();

	std::Array<Category*> *categories = new std::Array<Category*>();

	Section *catlist = data->getCategoryList();

	size_t catlist_size = catlist->getSize();

	mach_vm_address_t base = macho->getBase() - data->getMachO()->getAslrSlide();

	mach_vm_address_t header = reinterpret_cast<mach_vm_address_t>(macho->getMachHeader());

	mach_vm_address_t address = catlist->getAddress();

	uint64_t *categorylist = reinterpret_cast<uint64_t*>(header + catlist->getOffset());

	for(uint64_t off = 0; off < catlist_size; off = off + sizeof(uint64_t))
	{
		Category *cat;

		uint64_t category = macho->getBufferAddress((*(categorylist + off / sizeof(uint64_t)) & 0xFFFFFFFFFFF) - macho->getAslrSlide());

		if(!category)
		{
			category = header + (*(categorylist + off / sizeof(uint64_t))  & 0xFFFFFFFFFFF);
		}

		struct _objc_2_category *objc_category = reinterpret_cast<struct _objc_2_category*>(category);

		if(category)
		{
			if(!(objc_category->class_name & 0x4000000000000000))
			{
				cat = new Category(data, objc_category);

				categories->add(cat);
			}
		}
	}

	return categories;
}

std::Array<Protocol*>* parseProtocolList(ObjCData *data)
{
	mrk::UserMachO *macho = data->getMachO();

	std::Array<Protocol*> *protocols = new std::Array<Protocol*>();

	Section *protlist = data->getProtocolList();

	Section *protrefs = data->getProtoRefs();

	size_t protolist_size = protlist->getSize();

	mach_vm_address_t base = macho->getBase() - data->getMachO()->getAslrSlide();

	mach_vm_address_t header = reinterpret_cast<mach_vm_address_t>(macho->getMachHeader());

	mach_vm_address_t address = protlist->getAddress();

	uint64_t *protolist = reinterpret_cast<uint64_t*>(header + protlist->getOffset());

	for(uint64_t off = 0; off < protolist_size; off = off + sizeof(uint64_t))
	{
		Protocol *p;

		uint64_t proto = macho->getBufferAddress(((*(protolist + off / sizeof(uint64_t))) & 0xFFFFFFFFFFF) - macho->getAslrSlide());

		if(!proto)
		{
			proto = header + *(protolist + off / sizeof(uint64_t)) & 0xFFFFFFFFFFF;
		}

		struct _objc_2_class_protocol *objc_protocol = reinterpret_cast<struct _objc_2_class_protocol*>(proto);

		if(objc_protocol)
		{
			p = new Protocol(data, objc_protocol);

			protocols->add(p);
		}
	}

	return protocols;
}

void parseMethodList(ObjCData *metadata, ObjC *object, std::Array<Method*> *methodList, enum MethodType methtype, struct _objc_2_class_method_info *methodInfo)
{
	mrk::UserMachO *macho = metadata->getMachO();

	struct _objc_2_class_method_info *methods;

	off_t off;

	mach_vm_address_t selectors = macho->isDyldCache() ? ObjectiveC::findSelectorsBase(macho) : 0;

	if(!methodInfo)
	{
		return;
	}

	methods = reinterpret_cast<struct _objc_2_class_method_info*>(methodInfo);

	off = sizeof(struct _objc_2_class_method_info);

	char *type = "";
	char *prefix = "";

	switch(methtype)
	{
		case INSTANCE_METHOD:
			type = "Instance";
			prefix = "-";

			break;
		case CLASS_METHOD:
			type = "Class";
			prefix = "+";

			break;
		case OPT_INSTANCE_METHOD:
			type = "Optional Instance";
			prefix = "-";

			break;
		case OPT_CLASS_METHOD:
			type = "Optional Class";
			prefix = "+";

			break;
		default:
			break;
	}

	MAC_RK_LOG("\t\t\t%s Methods\n", type);

	for(int i = 0; i < methods->count; i++)
	{
		mach_vm_address_t pointer_to_name;
		mach_vm_address_t offset_to_name;

		uint8_t *q = reinterpret_cast<uint8_t*>(reinterpret_cast<uint8_t*>(methods) + off);

		if(macho->isDyldCache() && dynamic_cast<Protocol*>(object))
		{
			struct _objc_2_method *method = reinterpret_cast<struct _objc_2_method*>(q);

			Method *meth = new Method(object, method);

			MAC_RK_LOG("\t\t\t\t0x%08llx: %s%s\n", meth->getImpl(), prefix, meth->getName());

			methodList->add(meth);

			off += sizeof(struct _objc_2_method);

			continue;
		} else if(macho->isDyldCache())
		{
			struct _objc_2_class_method *method = reinterpret_cast<struct _objc_2_class_method*>(q);

			pointer_to_name = selectors + method->name;

			if(macho->getObjectiveCLibrary()->getBufferAddress(pointer_to_name))
			{
				Method *meth = new Method(object, method);

				methodList->add(meth);

				MAC_RK_LOG("\t\t\t\t0x%08llx: %s%s\n", meth->getImpl(), prefix, meth->getName());
			}

		} else
		{
			struct _objc_2_class_method *method = reinterpret_cast<struct _objc_2_class_method*>(q);

			pointer_to_name = reinterpret_cast<mach_vm_address_t>(macho->getMachHeader()) + reinterpret_cast<uint32_t>(method->name & 0xFFFFFF);

			offset_to_name = ((*(uint64_t*) pointer_to_name) & 0xFFFFFF);

			Method *meth = new Method(object, method);

			methodList->add(meth);

			MAC_RK_LOG("\t\t\t\t0x%08llx: %s%s\n", meth->getImpl(), prefix, meth->getName());
		}
	
		off += sizeof(struct _objc_2_class_method);
	}
}

void parsePropertyList(ObjCData *metadata, ObjC *object, std::Array<Property*> *propertyList, struct _objc_2_class_property_info *propertyInfo)
{
	struct _objc_2_class_property_info *properties;

	off_t off;

	if(!propertyInfo)
	{
		return;
	}

	properties = reinterpret_cast<struct _objc_2_class_property_info*>(propertyInfo);

	off = sizeof(struct _objc_2_class_property_info);

	MAC_RK_LOG("\t\t\tProperties\n");

	for(int i = 0; i < properties->count; i++)
	{
		struct _objc_2_class_property *property = reinterpret_cast<struct _objc_2_class_property*>(reinterpret_cast<uint8_t*>(properties) + off);

		Property *prop = new Property(object, property);

		propertyList->add(prop);

		MAC_RK_LOG("\t\t\t\t%s %s\n", prop->getAttributes(), prop->getName());

		off += sizeof(struct _objc_2_class_property);
	}
}
}

namespace ObjectiveC
{

ObjCClass::ObjCClass(ObjCData *metadata, struct _objc_2_class *c, bool metaclass)
{
	this->metadata = metadata;
	this->macho = metadata->getMachO();
	this->metaclass = metaclass;
	this->cls = c;
	this->super = NULL;

	this->data = reinterpret_cast<struct _objc_2_class_data*>((uint64_t)c->data & 0xFFFFFFF8);

	if(this->macho->sectionForOffset(reinterpret_cast<mach_vm_address_t>(this->data)))
	{
		this->data = reinterpret_cast<struct _objc_2_class_data*>((uint64_t) this->data + reinterpret_cast<mach_vm_address_t>(this->macho->getMachHeader()));
		this->name = reinterpret_cast<char*>((data->name & 0xFFFFFFFF) + reinterpret_cast<mach_vm_address_t>(this->macho->getMachHeader()));

		if(this->macho->sectionForOffset(reinterpret_cast<mach_vm_address_t>((data->name & 0xFFFFFFFF))))
		{
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
		}

	} else if(this->macho->sectionForAddress(reinterpret_cast<mach_vm_address_t>(((uint64_t) c->data & 0xFFFFFFFFF) - this->macho->getAslrSlide())))
	{
		this->data = reinterpret_cast<struct _objc_2_class_data*>(this->macho->getBufferAddress(((uint64_t)c->data & 0xFFFFFFFFF) - macho->getAslrSlide()));

		this->name = reinterpret_cast<char*>(macho->getBufferAddress(data->name - macho->getAslrSlide()));

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
	} else
	{
		this->name = NULL;
		this->data = NULL;
		this->isa = 0;
		this->superclass = 0;
		this->vtable = 0;
		this->cache = 0;

	}
}

Protocol::Protocol(ObjCData *data, struct _objc_2_class_protocol *prot)
{
	mrk::UserMachO *macho;

	struct _objc_2_class_method_info *instanceMethods;
	struct _objc_2_class_method_info *classMethods;

	struct _objc_2_class_method_info *optionalInstanceMethods;
	struct _objc_2_class_method_info *optionalClassMethods;

	struct _objc_2_class_property_info *instanceProperties;

	this->metadata = data;
	this->protocol = prot;

	macho = this->metadata->getMachO();

	if(macho->isDyldCache())
	{
		this->name = reinterpret_cast<char*>(macho->getBufferAddress(prot->name - macho->getAslrSlide()));

		printf("\t\t$OBJC_PROTOCOL_%s\n", this->name);

		if(prot->instance_methods)
		{
			instanceMethods = reinterpret_cast<struct _objc_2_class_method_info*>(macho->getBufferAddress(this->protocol->instance_methods - macho->getAslrSlide()));

			ObjectiveC::parseMethodList(this->metadata, this, this->getInstanceMethods(), INSTANCE_METHOD, instanceMethods);
		}

		if(prot->class_methods)
		{
			classMethods = reinterpret_cast<struct _objc_2_class_method_info*>(macho->getBufferAddress(this->protocol->class_methods - macho->getAslrSlide()));

			ObjectiveC::parseMethodList(this->metadata, this, this->getClassMethods(), CLASS_METHOD, classMethods);
		}

		if(prot->opt_instance_methods)
		{
			optionalInstanceMethods = reinterpret_cast<struct _objc_2_class_method_info*>(macho->getBufferAddress(this->protocol->opt_instance_methods - macho->getAslrSlide()));

			ObjectiveC::parseMethodList(this->metadata, this, this->getOptionalInstanceMethods(), OPT_INSTANCE_METHOD, optionalInstanceMethods);
		}
		
		if(prot->opt_class_methods)
		{
			optionalClassMethods = reinterpret_cast<struct _objc_2_class_method_info*>(macho->getBufferAddress(this->protocol->opt_class_methods - macho->getAslrSlide()));

			ObjectiveC::parseMethodList(this->metadata, this, this->getOptionalClassMethods(), OPT_CLASS_METHOD, optionalClassMethods);
		}

		if(prot->instance_properties)
		{
			instanceProperties = reinterpret_cast<struct _objc_2_class_property_info*>(macho->getBufferAddress(this->protocol->instance_properties - macho->getAslrSlide()));

			ObjectiveC::parsePropertyList(this->metadata, this, this->getInstanceProperties(), instanceProperties);
		}
	} else
	{
		this->name = reinterpret_cast<char*>((prot->name & 0xFFFFFFF) + reinterpret_cast<mach_vm_address_t>(macho->getMachHeader()));

		printf("\t\t$OBJC_PROTOCOL_%s\n", this->name);

		if(prot->instance_methods)
		{
			instanceMethods = reinterpret_cast<struct _objc_2_class_method_info*>((this->protocol->instance_methods & 0xFFFFFFF) + reinterpret_cast<mach_vm_address_t>(macho->getMachHeader()));

			ObjectiveC::parseMethodList(this->metadata, this, this->getInstanceMethods(), INSTANCE_METHOD, instanceMethods);
		}

		if(prot->class_methods)
		{
			classMethods = reinterpret_cast<struct _objc_2_class_method_info*>((this->protocol->class_methods & 0xFFFFFFF) + reinterpret_cast<mach_vm_address_t>(macho->getMachHeader()));

			ObjectiveC::parseMethodList(this->metadata, this, this->getClassMethods(), CLASS_METHOD, classMethods);
		}

		if(prot->opt_instance_methods)
		{
			optionalInstanceMethods = reinterpret_cast<struct _objc_2_class_method_info*>((this->protocol->opt_instance_methods & 0xFFFFFFF) + reinterpret_cast<mach_vm_address_t>(macho->getMachHeader()));
				
			ObjectiveC::parseMethodList(this->metadata, this, this->getOptionalInstanceMethods(), OPT_INSTANCE_METHOD, optionalInstanceMethods);
		}
		
		if(prot->opt_class_methods)
		{
			optionalClassMethods = reinterpret_cast<struct _objc_2_class_method_info*>((this->protocol->opt_class_methods & 0xFFFFFFF) + reinterpret_cast<mach_vm_address_t>(macho->getMachHeader()));

			ObjectiveC::parseMethodList(this->metadata, this, this->getOptionalClassMethods(), OPT_CLASS_METHOD, optionalClassMethods);
		}

		if(prot->instance_properties)
		{
			instanceProperties = reinterpret_cast<struct _objc_2_class_property_info*>((this->protocol->instance_properties & 0xFFFFFFF) + reinterpret_cast<mach_vm_address_t>(macho->getMachHeader()));

			ObjectiveC::parsePropertyList(this->metadata, this, this->getInstanceProperties(), instanceProperties);
		}
	}
}

Category::Category(ObjCData *data, struct _objc_2_category *cat)
{
	mrk::UserMachO *macho = data->getMachO();

	struct _objc_2_class_method_info *instanceMethods;
	struct _objc_2_class_method_info *classMethods;

	struct _objc_2_class_property_info *catProperties;

	this->metadata = data;
	this->category = cat;

	if(macho->isDyldCache())
	{
		this->name = reinterpret_cast<char*>(macho->getBufferAddress(cat->category_name - macho->getAslrSlide()));

		if(!name)
		{
			this->name = reinterpret_cast<char*>(macho->getObjectiveCLibrary()->getBufferAddress(cat->category_name - macho->getAslrSlide()));
		}

		ObjCClass *isa = data->getClassByName(this->name);

		if(isa)
		{
			this->class_name = isa->getName();
		} else
		{
			this->class_name = "UNKNOWN";
		}

		printf("\t\t$OBJC_CATEGORY_%s+%s\n", this->class_name, "extra");

		if(cat->instance_methods)
		{
			instanceMethods = reinterpret_cast<struct _objc_2_class_method_info*>(macho->getBufferAddress(this->category->instance_methods - macho->getAslrSlide()));

			ObjectiveC::parseMethodList(this->metadata, this, this->getInstanceMethods(), INSTANCE_METHOD, instanceMethods);
		}

		if(cat->class_methods)
		{
			classMethods = reinterpret_cast<struct _objc_2_class_method_info*>(macho->getBufferAddress(this->category->class_methods - macho->getAslrSlide()));

			ObjectiveC::parseMethodList(this->metadata, this, this->getClassMethods(), CLASS_METHOD, classMethods);
		}

		if(cat->properties)
		{
			catProperties = reinterpret_cast<struct _objc_2_class_property_info*>(macho->getBufferAddress(this->category->properties - macho->getAslrSlide()));

			ObjectiveC::parsePropertyList(this->metadata, this, this->getProperties(), catProperties);
		}

	} else
	{
		this->name = reinterpret_cast<char*>((cat->category_name & 0xFFFFFFF) + reinterpret_cast<mach_vm_address_t>(macho->getMachHeader()));

		if(!(cat->class_name & 0x4000000000000000))
		{
			mach_vm_address_t cls = ((cat->class_name & 0xFFFFFFF) + reinterpret_cast<mach_vm_address_t>(macho->getMachHeader()));
		
			ObjCClass *isa = data->getClassByIsa((mach_vm_address_t) cls);

			if(isa)
			{
				this->class_name = isa->getName();
			} else
			{
				this->class_name = "UNKNOWN";
			}
		} else
		{
			this->class_name = "UNKNOWN";
		}
		
		printf("\t\t$OBJC_CATEGORY_%s+%s\n", this->class_name, this->name);

		if(cat->instance_methods)
		{
			instanceMethods = reinterpret_cast<struct _objc_2_class_method_info*>((this->category->instance_methods & 0xFFFFFFF) + reinterpret_cast<mach_vm_address_t>(macho->getMachHeader()));

			ObjectiveC::parseMethodList(this->metadata, this, this->getInstanceMethods(), INSTANCE_METHOD, instanceMethods);
		}

		if(cat->class_methods)
		{
			classMethods = reinterpret_cast<struct _objc_2_class_method_info*>((this->category->class_methods & 0xFFFFFFF) + reinterpret_cast<mach_vm_address_t>(macho->getMachHeader()));

			ObjectiveC::parseMethodList(this->metadata, this, this->getClassMethods(), CLASS_METHOD, classMethods);
		}

		if(cat->properties)
		{
			catProperties = reinterpret_cast<struct _objc_2_class_property_info*>((this->category->properties & 0xFFFFFFF) + reinterpret_cast<mach_vm_address_t>(macho->getMachHeader()));

			ObjectiveC::parsePropertyList(this->metadata, this, this->getProperties(), catProperties);
		}
	}
}

Ivar::Ivar(ObjC *object, struct _objc_2_class_ivar *ivar)
{
	mrk::UserMachO *macho = object->getMetadata()->getMachO();

	if(macho->isDyldCache())
	{
		this->object = object;
		this->ivar = ivar;
		this->name = reinterpret_cast<char*>(macho->getBufferAddress(ivar->name - macho->getAslrSlide()));

		if(!name)
		{
			this->name = reinterpret_cast<char*>(macho->getObjectiveCLibrary()->getBufferAddress(ivar->name - macho->getAslrSlide()));
		}

		this->offset = ivar->offset - macho->getAslrSlide();
		this->type = ivar->type;
		this->size = ivar->size;

	} else
	{
		this->object = object;
		this->ivar = ivar;
		this->offset = reinterpret_cast<mach_vm_address_t>((ivar->offset & 0xFFFFFF) + reinterpret_cast<mach_vm_address_t>(macho->getMachHeader()));
		this->name = reinterpret_cast<char*>((ivar->name & 0xFFFFFF) + reinterpret_cast<char*>(macho->getMachHeader()));
		this->type = ivar->type;
		this->size = ivar->size;
	}
}

Property::Property(ObjC *object, struct _objc_2_class_property *property)
{
	mrk::UserMachO *macho = object->getMetadata()->getMachO();

	if(macho->isDyldCache())
	{
		this->object = object;
		this->property = property;
		this->name = reinterpret_cast<char*>(macho->getBufferAddress(property->name - macho->getAslrSlide()));

		if(!name)
		{
			this->name = reinterpret_cast<char*>(macho->getObjectiveCLibrary()->getBufferAddress(property->name - macho->getAslrSlide()));
		}

		this->attributes = reinterpret_cast<char*>(macho->getBufferAddress(property->attributes - macho->getAslrSlide()));

		if(!attributes)
		{
			this->attributes = reinterpret_cast<char*>(macho->getObjectiveCLibrary()->getBufferAddress(property->attributes - macho->getAslrSlide()));
		}

	} else
	{
		this->object = object;
		this->property = property;
		this->name = reinterpret_cast<char*>((property->name & 0xFFFFFF) + reinterpret_cast<char*>(macho->getMachHeader()));
		this->attributes = reinterpret_cast<char*>((property->attributes & 0xFFFFFF) + reinterpret_cast<char*>(macho->getMachHeader()));
	}
}

Method::Method(ObjC *object, struct _objc_2_class_method *method)
{
	mrk::UserMachO *macho = object->getMetadata()->getMachO();

	mach_vm_address_t selectors;

	mach_vm_address_t pointer_to_type;

	mach_vm_address_t pointer_to_name;

	this->object = object;
	this->method = method;

	selectors = ObjectiveC::findSelectorsBase(macho);

	if(macho->isDyldCache() && selectors)
	{
		pointer_to_name = selectors + method->name;

		pointer_to_name = macho->getObjectiveCLibrary()->getBufferAddress(pointer_to_name);

		this->name = reinterpret_cast<char*>(pointer_to_name);

		pointer_to_type = macho->offsetToAddress((mach_vm_address_t) &method->type - reinterpret_cast<mach_vm_address_t>(macho->getMachHeader()));

		pointer_to_type += method->type;

		pointer_to_type = macho->getBufferAddress(pointer_to_type);

		if(!pointer_to_type)
		{
			pointer_to_type = macho->getObjectiveCLibrary()->getBufferAddress(pointer_to_type);
		}

		this->type = pointer_to_type;

		this->impl = reinterpret_cast<mach_vm_address_t>(method->imp + macho->offsetToAddress(reinterpret_cast<mach_vm_address_t>(&method->imp) - reinterpret_cast<mach_vm_address_t>(macho->getMachHeader())));
	} else
	{
		if(dynamic_cast<ObjCClass*>(this->object) || dynamic_cast<Category*>(this->object))
		{
			pointer_to_name = reinterpret_cast<mach_vm_address_t>(&method->name) + reinterpret_cast<uint32_t>(method->name & 0xFFFFFF);

			pointer_to_name = reinterpret_cast<mach_vm_address_t>(macho->getMachHeader()) + ((*(uint64_t*) pointer_to_name) & 0xFFFFFF);
		} else
		{
			pointer_to_name = reinterpret_cast<mach_vm_address_t>(macho->getMachHeader()) + reinterpret_cast<uint32_t>(method->name & 0xFFFFFF);
		}

		this->name = reinterpret_cast<char*>(pointer_to_name);
		this->impl = reinterpret_cast<mach_vm_address_t>((method->imp & 0xFFFFFF) + reinterpret_cast<mach_vm_address_t>(macho->getMachHeader()));
		this->type = method->type;
	}
}

Method::Method(ObjC *object, struct _objc_2_method *method)
{
	mrk::UserMachO *macho = object->getMetadata()->getMachO();

	this->object = object;
	
	this->name = reinterpret_cast<char*>(macho->getBufferAddress(method->name - macho->getAslrSlide()));

	if(!name)
	{
		this->name = reinterpret_cast<char*>(macho->getObjectiveCLibrary()->getBufferAddress(method->name - macho->getAslrSlide()));
	}

	this->impl = method->imp ? method->imp - macho->getAslrSlide() : 0;
	this->type = macho->getBufferAddress(method->type - macho->getAslrSlide());
}
}

#ifdef __arm64__

#include <arm64/PatchFinder_arm64.hpp>
#include <arm64/Isa_arm64.hpp>

#elif

#include <x86_64/PatchFinder_x86_64.hpp>
#include <x86_64/Isa_x86_64.hpp>

#endif

namespace ObjectiveC
{
	
mach_vm_address_t findSelectorsBase(mrk::UserMachO *macho)
{
	mach_vm_address_t selectors;

	mach_vm_address_t method_getName;

	mrk::UserMachO *libobjc;

	Symbol *symbol;

	libobjc = macho->getObjectiveCLibrary();

	if(!libobjc)
	{
		return 0;
	}

	symbol = libobjc->getSymbolByName("_method_getName");

	if(!symbol)
	{
		return 0;
	}

	method_getName = symbol->getAddress();

	if(!method_getName)
	{
		return 0;
	}

	mach_vm_address_t start = libobjc->getBufferAddress(method_getName);

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
}

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

	mach_vm_address_t selectors = 0;

	off_t off;

	c = this->cls;

	d = this->data;

	if(!d->methods)
	{
		return;
	}

	if(this->macho->isDyldCache())
	{
		selectors = ObjectiveC::findSelectorsBase(this->macho);
	}
	
	methods = NULL;
	
	if(this->macho->isDyldCache())
	{
		methods = reinterpret_cast<struct _objc_2_class_method_info*>(this->macho->getBufferAddress((d->methods & 0xFFFFFFFFFF)  - this->macho->getAslrSlide()));
	} else
	{
		methods = reinterpret_cast<struct _objc_2_class_method_info*>((d->methods & 0xFFFFFFFFF) + reinterpret_cast<mach_vm_address_t>(this->macho->getMachHeader()));
	}

	if(!methods)
	{
		return;
	}

	off = sizeof(struct _objc_2_class_method_info);

	MAC_RK_LOG("\t\t\tMethods\n");

	for(int i = 0; i < methods->count; i++)
	{
		struct _objc_2_class_method *method = reinterpret_cast<struct _objc_2_class_method*>(reinterpret_cast<uint8_t*>(methods) + off);

		mach_vm_address_t pointer_to_name;
		mach_vm_address_t offset_to_name;

		if(selectors)
		{
			pointer_to_name = selectors + method->name;

			if(this->macho->getObjectiveCLibrary()->getBufferAddress(pointer_to_name))
			{
				Method *meth = new Method(this, method);

				this->methods.add(meth);

				if(metaclass)
				{
					MAC_RK_LOG("\t\t\t\t0x%08llx: +%s\n", meth->getImpl(), meth->getName());
				} else
				{
					MAC_RK_LOG("\t\t\t\t0x%08llx: -%s\n", meth->getImpl(), meth->getName());
				}
			}

		} else
		{
			pointer_to_name = reinterpret_cast<mach_vm_address_t>(&method->name) + reinterpret_cast<uint32_t>(method->name & 0xFFFFFFFF);

			offset_to_name = ((*(uint64_t*) pointer_to_name));

			if(offset_to_name)
			{
				Section *sect = this->macho->sectionForOffset(offset_to_name);

				if(sect && strcmp(sect->getSectionName(), "__objc_methname") == 0)
				{
					Method *meth = new Method(this, method);

					this->methods.add(meth);

					if(metaclass)
					{
						MAC_RK_LOG("\t\t\t\t0x%08llx: +%s\n", meth->getImpl(), meth->getName());
					} else
					{
						MAC_RK_LOG("\t\t\t\t0x%08llx: -%s\n", meth->getImpl(), meth->getName());
					}
				}
			}
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

	ivars = NULL;

	if(this->macho->isDyldCache())
	{
		ivars = reinterpret_cast<struct _objc_2_class_ivar_info*>(this->macho->getBufferAddress(d->ivars  - this->macho->getAslrSlide()));
	} else
	{
		ivars = reinterpret_cast<struct _objc_2_class_ivar_info*>((d->ivars & 0xFFFFFFFF) + reinterpret_cast<mach_vm_address_t>(this->macho->getMachHeader()));
	}

	if(!ivars)
	{
		return;
	}

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

	properties = NULL;

	if(this->macho->isDyldCache())
	{
		properties = reinterpret_cast<struct _objc_2_class_property_info*>(this->macho->getBufferAddress(d->properties  - this->macho->getAslrSlide()));
	} else
	{
		properties = reinterpret_cast<struct _objc_2_class_property_info*>((d->properties & 0xFFFFFFFF) + reinterpret_cast<mach_vm_address_t>(this->macho->getMachHeader()));
	}

	if(!properties)
	{
		return;
	}

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

	assert(this->selrefs);

	assert(this->ivar);
	assert(this->objc_data);

	this->classes = parseClassList(this);

	if(this->protolist)
		this->protocols = parseProtocolList(this);

	if(this->catlist)
		this->categories = parseCategoryList(this);

	if(!this->macho->isDyldCache())
	{
		// do not parse categories and protocols when dyld cache is being parsed
		// all of the class pointers will be invalid because they will exist in other libraries
		// we aren't going to need categories during runtime anyways
	}
}

ObjCClass* ObjCData::getClassByName(char *classname)
{
	for(int i = 0; i < this->classes->getSize(); i++)
	{
		ObjCClass *cls = this->classes->get(i);

		if(cls->getName() && strcmp(cls->getName(), classname) == 0)
		{
			return cls;
		}
	}

	return NULL;
}

ObjCClass* ObjCData::getClassByIsa(mach_vm_address_t isa)
{
	for(int i = 0; i < this->classes->getSize(); i++)
	{
		ObjCClass *cls = this->classes->get(i);

		if(reinterpret_cast<mach_vm_address_t>(cls->getClass()) == isa)
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