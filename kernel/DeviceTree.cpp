#include "Kernel.hpp"
#include "Log.hpp"

#include "DeviceTree.hpp"

#ifdef __arm64__

#include "PatchFinder_arm64.hpp"

#include <libkern/libkern.h>
#include <pexpert/pexpert.h>

using namespace xnu;

bool is_ascii(char *c, size_t len)
{
	uint32_t zeros = 0;

	for (size_t i = 0; i < len; i++)
	{
		if(c[i] < 0)
			return false;
		else if(c[i] == 0)
			zeros++;
	}

	return zeros < 3 ? true : false;
}

bool DeviceTree::iterateNodeProperties(void **data, 
							 			void *data_end,
							 			uint32_t *depth,
										DeviceTreeNode *node,
										dt_property_callback_t prop_cb,
										bool *success)
{
	bool cb;

	uint8_t *p = (uint8_t*) *data;
	uint8_t *end = data_end;

	uint32_t n_props = node->n_properties;
	uint32_t n_children = node->n_children;

	*success = false;

	for(int i = 0; i < n_props; i++)
	{
		DeviceTreeProperty *prop = (DeviceTreeProperty*) p;

		if(p + sizeof(*prop) > end)
			return false;

		if(prop->name[sizeof(prop->name) - 1] != 0)
			return false;

		uint32_t prop_size = prop->size & ~0x80000000;
		uint32_t padded_size = (prop_size + 0x3) & ~0x3;

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

size_t DeviceTree::getNodeSize(uint32_t depth, DeviceTreeNode *node)
{
	void *dt = this->getDeviceTree();
	
	size_t dt_size = this->getDeviceTreeSize();

	bool ok;
	bool success;

	void *dt_end = (void*) ((uint8_t*) dt + dt_size);

	uint8_t *p = (uint8_t*) node;

	p += sizeof(*node);

	if(p > (uint8_t*) dt_end)
		return 0;

	uint8_t *end = p;

	uint32_t n_props = node->n_properties;
	uint32_t n_children = node->n_children;

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

bool DeviceTree::iterateNode(void **data, 
						     void *data_end,
						     uint32_t *depth,
						     dt_node_callback_t node_cb,
						     dt_property_callback_t prop_cb,
						     bool *success)
{
	bool ok;
	bool cb;

	uint8_t *p = (uint8_t*) *data;
	uint8_t *end = data_end;

	DeviceTreeNode *node = (DeviceTreeNode*) p;

	if(node_cb)
	{
		cb = node_cb(*depth, (void*) node, end - (uint8_t*) node);

		if(cb)
		{
			*success = true;

			return true;
		}
	}

	p += sizeof(*node);

	if(p > end)
		return false;

	uint32_t n_props = node->n_properties;
	uint32_t n_children = node->n_children;

	ok = DeviceTree::iterateNodeProperties((void**) &p, data_end, depth, node, prop_cb, success);

	*data = p;

	if(*success)
		return false;

	if(!ok)
		return false;

	for(uint32_t i = 0; i < n_children; i++)
	{
		uint32_t child_depth = *depth + 1;

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

DeviceTreeNode* DeviceTree::findNode(char *nodename, uint32_t *depth)
{
	void *dt = this->getDeviceTree();
	
	size_t dt_size = this->getDeviceTreeSize();

	uint8_t *root = (uint8_t*) dt;

	*depth = 0;

	dt_property_callback_t prop_cb =  ^bool (uint32_t ndepth, void *property, uint32_t size
	{
		DeviceTreeProperty *prop = (DeviceTreeProperty*) property;

		if(strcmp(prop->name, "name") == 0)
			return true;

		return false;
	};

	dt_node_callback_t node_cb =  ^bool (uint32_t ndepth, void *dtnode, uint32_t size)
	{
		bool ok;
		bool success;

		DeviceTreeNode *node = (DeviceTreeNode*) dtnode;

		uint8_t *p = (uint8_t*) node;

		p += sizeof(*node);

		uint32_t node_size = DeviceTree::getNodeSize(dt, dt_size, ndepth, node);

		ok = DeviceTree::iterateNodeProperties((void**) &p, 
										  (void*) ((uint8_t*) node + node_size),
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

	bool ok;
	bool success = false;

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
	void *dt = this->getDeviceTree();
	
	size_t dt_size = this->getDeviceTreeSize();

	DeviceTree *node;

	bool ok;
	bool success;

	uint32_t node_size;
	uint32_t depth = 0;

	node = DeviceTree::findNode(dt, dt_size, nodename, &depth);
	node_size = DeviceTree::getNodeSize(dt, dt_size, depth, node);

	if(node)
	{
		dt_property_callback_t prop_cb = 
				^bool (uint32_t depth, void *property, uint32_t size)
				{
						DeviceTreeProperty *prop = (DeviceTreeProperty*) property;

						if(strcmp(prop->name, propname) == 0)
							return true;

						return false;
				};

		success = false;

		ok = DeviceTree::iterateNode((void**) &node, (void*)((uint8_t*)node + node_size), &depth, NULL, prop_cb, &success);

		if(success)
		{
			DeviceTreeProperty *prop = (DeviceTreeProperty*) node;

			MAC_RK_LOG("%s %s ", nodename, prop->name);

			DeviceTree::printData((uint8_t*) &prop->val, prop->size);

			return prop;
		}
	}

	return NULL;
}

void DeviceTree::printData(uint8_t *prop_data, uint32_t prop_size)
{
	if(is_ascii((char*) prop_data, prop_size) && !prop_data[strlen((char*) prop_data)])
		MAC_RK_LOG("%s ", (char*) prop_data);
	else if(prop_size == sizeof(uint64_t))
		MAC_RK_LOG("0x%llx ", *(uint64_t*) prop_data);
	else if(prop_size == sizeof(uint32_t))
		MAC_RK_LOG("0x%x ", *(uint32_t*) prop_data);
	else
	{
		for(uint32_t j = 0; j < prop_size; j++)
			MAC_RK_LOG("%x ", prop_data[j]);
	}

	MAC_RK_LOG("\n");
}

void DeviceTree::printNode(char *nodename)
{
	void *dt = this->getDeviceTree();
	
	size_t dt_size = this->getDeviceTreeSize();

	DeviceTree *node;

	uint32_t depth;

	node = DeviceTree::findNode(dt, dt_size, nodename, &depth);

	if(node)
	{
		uint32_t node_size = DeviceTree::getNodeSize(dt, dt_size, depth, node);

		DeviceTree::print((void*) node, node_size);
	}
}

void DeviceTree::print()
{
	void *dt = this->getDeviceTree();
	
	size_t dt_size = this->getDeviceTreeSize();

	bool success = false;

	uint32_t depth = 0;

	uint8_t *root = (uint8_t*) dt;

	dt_property_callback_t prop_cb =  ^bool (uint32_t depth, void *property, uint32_t size)
	{
		dt_prop *prop = (DeviceTreeProperty*) property;

		uint8_t *prop_data = (uint8_t*) &prop->val;
		uint32_t prop_size = prop->size;

		for(uint32_t j = 0; j < depth; j++)
			MAC_RK_LOG("\t");

		MAC_RK_LOG("%s ", prop->name);

		dt_print_data(prop_data, prop_size);

		return false;
	};

	DeviceTree::iterateNode((void**) &root, (void*)(root + dt_size), &depth, NULL, prop_cb, &success);
}

void* DeviceTree::dump()
{
	void *dt = this->getDeviceTree();

	size_t dt_size = this->getDeviceTreeSize();

	void *device_tree;

	uint64_t sz = dt_size;
	uint64_t off = 0;

	device_tree = IOMalloc(sz);

	while(sz > 0)
	{
		uint64_t to_read = sz / 0x1000 > 0 ? 0x1000 : sz;

		if(!(uint64_t*) (dt + off, device_tree + off, to_read))
		{
			MAC_RK_LOG("Failed to dump device tree at 0x%llx\n", dt + off);

			goto exit;
		}

		sz -= to_read;
		off += to_read;
	}

	return device_tree;

exit:
	IOFree(device_tree);

	return NULL;
}


PE_state_t PE_state(Kernel *kernel)
{
	uintptr_t device_tree;

	uint64_t deviceTreeHead;
	uint32_t deviceTreeSize;

	mach_vm_address_t PE_state;
	mach_vm_address_t boot_args;
	
	mach_vm_address_t __ZN16IOPlatformExpert14getConsoleInfoEP8PE_Video = kernel->getSymbolAddressByName("__ZN16IOPlatformExpert14getConsoleInfoEP8PE_Video");

	mach_vm_address_t adrp_ins = Arch::arm64::PatchFinder::step64(kernel->getMachO(), __ZN16IOPlatformExpert14getConsoleInfoEP8PE_Video, 0xF0, reinterpret_cast<bool(*)(uint32_t*)>(is_adrp), -1, -1);

	adr_t adrp = *(adr_t*) adrp_ins;

	uint64_t page = (((adrp.immhi << 2) | adrp.immlo)) << 12;

	mach_vm_address_t add_ins = Arch::arm64::PatchFinder::step64(macho, adrp_ins, 8, reinterpret_cast<bool(*)(uint32_t*)>(is_add_imm), NO_REG, NO_REG);

	add_imm_t add = *(add_imm_t*) add_ins;

	uint64_t page_offset = add.imm << (add.sh ? 12 : 0);

	PE_state = ((adrp_ins ~0x3FFF) + (page | page_offset)) & ~((uint64_t) 0xF);

	return static_cast<PE_state_t*>(PE_state);
}

void* getDeviceTreeHead(Kernel *kernel)
{
	PE_state_t *PE_State = findPE_State(kernel);

	void *deviceTreeHead = static_cast<void*>(kernel->read64(static_cast<mach_vm_address_t>(PE_State) + offsetof(struct PE_state, deviceTreeHead)));
	
	return deviceTreeHead;
}

size_t getDeviceTreeSize(Kernel *kernel)
{
	mach_vm_address_t PE_State = findPE_State(kernel);

	size_t deviceTreeSize = kernel->read32(PE_state + offsetof(struct PE_state, deviceTreeSize));

	return deviceTreeSize;
}

#endif