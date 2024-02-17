#ifdef __arm64__

#include <arm64/PatchFinder_arm64.hpp>

#include "Kernel.hpp"
#include "MachO.hpp"
#include "DeviceTree.hpp"

#include "Log.hpp"

using namespace xnu;

Bool is_ascii(char *c, Size len)
{
	UInt32 zeros = 0;

	for (Size i = 0; i < len; i++)
	{
		if(c[i] < 0)
			return false;
		else if(c[i] == 0)
			zeros++;
	}

	return zeros < 3 ? true : false;
}

Bool DeviceTree::iterateNodeProperties(void **data, 
							 			void *data_end,
							 			UInt32 *depth,
										DeviceTreeNode *node,
										dt_property_callback_t prop_cb,
										Bool *success)
{
	Bool cb;

	UInt8 *p = (UInt8*) data;
	UInt8 *end = (UInt8*) data_end;

	UInt32 n_props = node->n_properties;
	UInt32 n_children = node->n_children;

	*success = false;

	for(int i = 0; i < n_props; i++)
	{
		DeviceTreeProperty *prop = (DeviceTreeProperty*) p;

		if(p + sizeof(*prop) > end)
			return false;

		if(prop->name[sizeof(prop->name) - 1] != 0)
			return false;

		UInt32 prop_size = prop->size & ~0x80000000;
		UInt32 padded_size = (prop_size + 0x3) & ~0x3;

		if(p + padded_size > end)
		{
			if(p - padded_size + prop_size == end)
				p = end;
			else
				return false;
		}

		if(prop_cb)
		{
			cb = prop_cb(*depth + 1, (void*) prop, prop_size);

			if(cb)
			{
				*success = true;
				*data = p;

				return true;
			}
		}

		p += sizeof(*prop);
		p += padded_size;
	}

	*data = p;

	return true;
}

Size DeviceTree::getNodeSize(UInt32 depth, DeviceTreeNode *node)
{
	UInt8 *dt = getAs<UInt8*>();
	
	Size dt_size = getSize();

	Bool ok;
	Bool success;

	void *dt_end = (void*) ((UInt8*) dt + dt_size);

	UInt8 *p = (UInt8*) node;

	p += sizeof(*node);

	if(p > (UInt8*) dt_end)
		return 0;

	UInt8 *end = p;

	UInt32 n_props = node->n_properties;
	UInt32 n_children = node->n_children;

	ok = DeviceTree::iterateNodeProperties((void**) &end, 
						 			dt_end,
						 			&depth,
									node,
									NULL,
									&success);

	if(ok)
		return (end - p) + sizeof(DeviceTreeNode);

	return 0;
}

Bool DeviceTree::iterateNode(void **data, 
						     void *data_end,
						     UInt32 *depth,
						     dt_node_callback_t node_cb,
						     dt_property_callback_t prop_cb,
						     Bool *success)
{
	Bool ok;
	Bool cb;

	UInt8 *p = (UInt8*) *data;
	UInt8 *end = (UInt8*) data_end;

	DeviceTreeNode *node = (DeviceTreeNode*) p;

	if(node_cb)
	{
		cb = node_cb(*depth, (void*) node, end - (UInt8*) node);

		if(cb)
		{
			*success = true;

			return true;
		}
	}

	p += sizeof(*node);

	if(p > end)
		return false;

	UInt32 n_props = node->n_properties;
	UInt32 n_children = node->n_children;

	ok = DeviceTree::iterateNodeProperties((void**) &p, data_end, depth, node, prop_cb, success);

	*data = p;

	if(*success)
		return false;

	if(!ok)
		return false;

	for(UInt32 i = 0; i < n_children; i++)
	{
		UInt32 child_depth = *depth + 1;

		ok = DeviceTree::iterateNode(data, data_end, &child_depth, node_cb, prop_cb, success);

		if(*success)
		{
			*depth = child_depth;

			return false;
		}

		if(!ok)
			return false;
	}

	return true;
}

DeviceTreeNode* DeviceTree::findNode(char *nodename, UInt32 *depth)
{
	UInt8 *dt = getAs<UInt8*>();
	
	Size dt_size = getSize();

	UInt8 *root = (UInt8*) dt;

	*depth = 0;

	dt_property_callback_t prop_cb =  ^Bool (UInt32 ndepth, void *property, UInt32 size)
	{
		DeviceTreeProperty *prop = (DeviceTreeProperty*) property;

		if(strcmp(prop->name, "name") == 0)
			return true;

		return false;
	};

	dt_node_callback_t node_cb =  ^Bool (UInt32 ndepth, void *dtnode, UInt32 size)
	{
		Bool ok;
		Bool success;

		DeviceTreeNode *node = (DeviceTreeNode*) dtnode;

		UInt8 *p = (UInt8*) node;

		p += sizeof(*node);

		UInt32 node_size = DeviceTree::getNodeSize(ndepth, node);

		ok = DeviceTree::iterateNodeProperties((void**) &p, 
										  (void*) ((UInt8*) node + node_size),
										  &ndepth,
										  node,
										  prop_cb,
										  &success);

		if(ok && success)
		{
			DeviceTreeProperty *prop = (DeviceTreeProperty*) p;

			if(strcmp(nodename, (char*) &prop->val) == 0)
				return true;
		}

		return false;
	};

	Bool ok;
	Bool success = false;

	ok = DeviceTree::iterateNode((void**) &root, (void*)(root + dt_size), depth, node_cb, NULL, &success);

	if(success)
	{
		DeviceTreeNode *node = (DeviceTreeNode*) root;

		return node;
	}

	return NULL;
}

DeviceTreeProperty* DeviceTree::findProperty(char *nodename, char *propname)
{
	UInt8 *dt = getAs<UInt8*>();
	
	Size dt_size = getSize();

	DeviceTreeNode *node;

	Bool ok;
	Bool success;

	UInt32 node_size;
	UInt32 depth = 0;

	node = DeviceTree::findNode(nodename, &depth);

	node_size = DeviceTree::getNodeSize(depth, node);

	if(node)
	{
		dt_property_callback_t prop_cb = 
				^Bool (UInt32 depth, void *property, UInt32 size)
				{
						DeviceTreeProperty *prop = (DeviceTreeProperty*) property;

						if(strcmp(prop->name, propname) == 0)
							return true;

						return false;
				};

		success = false;

		ok = DeviceTree::iterateNode((void**) &node, (void*)((UInt8*)node + node_size), &depth, NULL, prop_cb, &success);

		if(success)
		{
			DeviceTreeProperty *prop = (DeviceTreeProperty*) node;

			MAC_RK_LOG("%s %s ", nodename, prop->name);

			DeviceTree::printData((UInt8*) &prop->val, prop->size);

			return prop;
		}
	}

	return NULL;
}

void DeviceTree::printData(UInt8 *prop_data, UInt32 prop_size)
{
	if(is_ascii((char*) prop_data, prop_size) && !prop_data[strlen((char*) prop_data)])
	{
		MAC_RK_LOG("%s ", (char*) prop_data);
	}
	else if(prop_size == sizeof(UInt64))
	{
		MAC_RK_LOG("0x%llx ", *(UInt64*) prop_data);
	}
	else if(prop_size == sizeof(UInt32))
	{
		MAC_RK_LOG("0x%x ", *(UInt32*) prop_data);
	}
	else
	{
		for(UInt32 j = 0; j < prop_size; j++)
			MAC_RK_LOG("%x ", prop_data[j]);
	}

	MAC_RK_LOG("\n");
}

void DeviceTree::printNode(char *nodename)
{
	UInt8 *dt = getAs<UInt8*>();
	
	Size dt_size = getSize();

	DeviceTreeNode *node;

	UInt32 depth;

	node = DeviceTree::findNode(nodename, &depth);

	if(node)
	{
		Size node_size = DeviceTree::getNodeSize(depth, node);

		DeviceTree::print(node, node_size);
	}
}

void DeviceTree::print(DeviceTreeNode *node, Size node_size)
{
	Bool success = false;

	UInt32 depth = 0;

	UInt8 *root = (UInt8*) node;

	Size sz = node_size;

	dt_property_callback_t prop_cb =  ^Bool (UInt32 depth, void *property, UInt32 size)
	{
		DeviceTreeProperty *prop = (DeviceTreeProperty*) property;

		UInt8 *prop_data = (UInt8*) &prop->val;
		UInt32 prop_size = prop->size;

		for(UInt32 j = 0; j < depth; j++)
			MAC_RK_LOG("\t");

		MAC_RK_LOG("%s ", prop->name);

		printData(prop_data, prop_size);

		return false;
	};

	DeviceTree::iterateNode((void**) &root, (void*)(root + sz), &depth, NULL, prop_cb, &success);
}

template<typename T>
T DeviceTree::dump()
{
	UInt8 *dt = getAs<UInt8*>();

	Size dt_size = getSize();

	UInt8 *device_tree;

	UInt64 sz = dt_size;
	UInt64 off = 0;

	device_tree = new UInt8[sz];

	while(sz > 0)
	{
		UInt64 to_read = sz / 0x1000 > 0 ? 0x1000 : sz;

		if(!(UInt64*) (dt + off, device_tree + off, to_read))
		{
			MAC_RK_LOG("Failed to dump device tree at 0x%llx\n", dt + off);

			return NULL;
		}

		sz -= to_read;
		off += to_read;
	}

	return reinterpret_cast<T>(device_tree);
}


PE_state_t* platformExpertState(xnu::Kernel *kernel)
{
	uintptr_t device_tree;

	UInt64 deviceTreeHead;

	UInt32 deviceTreeSize;

	xnu::Mach::VmAddress PE_state;
	xnu::Mach::VmAddress boot_args;
	
	xnu::Mach::VmAddress __ZN16IOPlatformExpert14getConsoleInfoEP8PE_Video = kernel->getSymbolAddressByName("__ZN16IOPlatformExpert14getConsoleInfoEP8PE_Video");

	xnu::Mach::VmAddress adrp_ins = Arch::arm64::PatchFinder::step64(kernel->getMachO(), __ZN16IOPlatformExpert14getConsoleInfoEP8PE_Video, 0xF0, reinterpret_cast<bool(*)(UInt32*)>(Arch::arm64::is_adrp), -1, -1);

	using namespace Arch::arm64;

	adr_t adrp = *(adr_t*) adrp_ins;

	UInt64 page = (((adrp.immhi << 2) | adrp.immlo)) << 12;

	xnu::Mach::VmAddress add_ins = Arch::arm64::PatchFinder::step64(kernel->getMachO(), adrp_ins, 8, reinterpret_cast<bool(*)(UInt32*)>(Arch::arm64::is_add_imm), NO_REG, NO_REG);

	add_imm_t add = *(add_imm_t*) add_ins;

	UInt64 page_offset = add.imm << (add.sh ? 12 : 0);

	PE_state = ((adrp_ins & ~0x3FFF) + (page | page_offset)) & ~((UInt64) 0xF);

	return reinterpret_cast<PE_state_t*>(PE_state);
}

#endif