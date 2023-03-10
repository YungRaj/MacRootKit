#include "IOKit.hpp"

#include "APIUtil.hpp"

#include "Log.hpp"

namespace IOKit
{
	static constexpr size_t bruteMax {40000000};
	
	OSSerialize *getProperty(IORegistryEntry *entry, const char *property)
	{
		IORegistryEntry *value = OSDynamicCast(IORegistryEntry, entry->getProperty(property));

		if (value)
		{
			OSSerialize *s = OSSerialize::withCapacity(PAGE_SIZE);

			if (value->serialize(s))
			{
				return s;
			} else
			{
				MAC_RK_LOG("failed to serialise %s property", property);

				s->release();
			}
		} else
		{
			MAC_RK_LOG("failed to get %s property", property);
		}

		return NULL;
	}

	bool awaitPublishing(IORegistryEntry *obj)
	{
		size_t counter = 0;

		while (counter < 256)
		{
			if (obj->inPlane(gIOServicePlane))
			{
				MAC_RK_LOG("pci device %s is in service plane %lu", obj->getName(), counter);
				
				return true;
			}

			MAC_RK_LOG("pci device %s is not in service plane %lu, polling", obj->getName(), counter);
			
			++counter;

			IOSleep(20);
		}

		MAC_RK_LOG("found dead pci device %s", obj->getName());
		
		return false;
	}

	uint32_t readPCIConfigValue(IORegistryEntry *service, uint32_t reg, uint32_t space, uint32_t size)
	{
		if (!awaitPublishing(service))
			return 0xffffffff;

		auto read32 = reinterpret_cast<t_PCIConfigRead32 **>(service)[0][PCIConfigOffset::ConfigRead32];
		auto read16 = reinterpret_cast<t_PCIConfigRead16 **>(service)[0][PCIConfigOffset::ConfigRead16];
		auto read8  = reinterpret_cast<t_PCIConfigRead8  **>(service)[0][PCIConfigOffset::ConfigRead8];

		if (space == 0) {
			space = getMember<uint32_t>(service, 0xA8);
			MAC_RK_LOG("read pci config discovered %s space to be 0x%08X", service->getName(), space);
		}

		if (size != 0) {
			switch (size) {
				case 8:
					return read8(service, space, reg);
				case 16:
					return read16(service, space, reg);
				default: /* assume 32-bit otherwise */
					return read32(service, space, reg);
			}
		}

		switch (reg) {
			case kIOPCIConfigVendorID:
				return read16(service, space, reg);
			case kIOPCIConfigDeviceID:
				return read16(service, space, reg);
			case kIOPCIConfigCommand:
				return read16(service, space, reg);
			case kIOPCIConfigStatus:
				return read16(service, space, reg);
			case kIOPCIConfigRevisionID:
				return read8(service, space, reg);
			case kIOPCIConfigClassCode:
				return read32(service, space, reg);
			case kIOPCIConfigCacheLineSize:
				return read8(service, space, reg);
			case kIOPCIConfigLatencyTimer:
				return read8(service, space, reg);
			case kIOPCIConfigHeaderType:
				return read8(service, space, reg);
			case kIOPCIConfigBIST:
				return read8(service, space, reg);
			case kIOPCIConfigBaseAddress0:
				return read32(service, space, reg);
			case kIOPCIConfigBaseAddress1:
				return read32(service, space, reg);
			case kIOPCIConfigBaseAddress2:
				return read32(service, space, reg);
			case kIOPCIConfigBaseAddress3:
				return read32(service, space, reg);
			case kIOPCIConfigBaseAddress4:
				return read32(service, space, reg);
			case kIOPCIConfigBaseAddress5:
				return read32(service, space, reg);
			case kIOPCIConfigCardBusCISPtr:
				return read32(service, space, reg);
			case kIOPCIConfigSubSystemVendorID:
				return read16(service, space, reg);
			case kIOPCIConfigSubSystemID:
				return read16(service, space, reg);
			case kIOPCIConfigExpansionROMBase:
				return read32(service, space, reg);
			case kIOPCIConfigCapabilitiesPtr:
				return read32(service, space, reg);
			case kIOPCIConfigInterruptLine:
				return read8(service, space, reg);
			case kIOPCIConfigInterruptPin:
				return read8(service, space, reg);
			case kIOPCIConfigMinimumGrant:
				return read8(service, space, reg);
			case kIOPCIConfigMaximumLatency:
				return read8(service, space, reg);
			default:
				return read32(service, space, reg);
		}
	}

	void getDeviceAddress(IORegistryEntry *service, uint8_t &bus, uint8_t &device, uint8_t &function)
	{
		auto getBus = reinterpret_cast<t_PCIConfigGetBusNumber **>(service)[0][PCIConfigOffset::GetBusNumber];
		auto getDevice = reinterpret_cast<t_PCIConfigGetDeviceNumber **>(service)[0][PCIConfigOffset::GetDeviceNumber];
		auto getFunction = reinterpret_cast<t_PCIConfigGetFunctionNumber **>(service)[0][PCIConfigOffset::GetFunctionNumber];

		bus = getBus(service);
		device = getDevice(service);
		function = getFunction(service);
	}

	IORegistryEntry *findEntryByPrefix(const char *path, const char *prefix, const IORegistryPlane *plane, bool (*proc)(void *, IORegistryEntry *), bool brute, void *user)
	{
		IORegistryEntry *entry = IORegistryEntry::fromPath(path, plane);

		if (entry)
		{
			auto res = findEntryByPrefix(entry, prefix, plane, proc, brute, user);
			entry->release();
			return res;
		}

		MAC_RK_LOG("failed to get %s entry", path);

		return NULL;
	}


	IORegistryEntry *findEntryByPrefix(IORegistryEntry *entry, const char *prefix, const IORegistryPlane *plane, bool (*proc)(void *, IORegistryEntry *), bool brute, void *user)
	{
		bool found = false;

		IORegistryEntry *res = NULL;

		size_t bruteCount = 0;

		do
		{
			bruteCount++;
			
			auto iterator = entry->getChildIterator(plane);

			if(iterator)
			{
				size_t len = strlen(prefix);
				
				while ((res = OSDynamicCast(IORegistryEntry, iterator->getNextObject())) != NULL)
				{
					const char *resname = res->getName();

					if (resname && !strncmp(prefix, resname, len))
					{
						found = proc ? proc(user, res) : true;
						
						if (found)
						{
							if (bruteCount > 1)
								MAC_RK_LOG("bruted %s value in %lu attempts", prefix, bruteCount);
							
							if (!proc)
								break;
						}
					}
				}

				iterator->release();

			} else
			{
				MAC_RK_LOG("failed to iterate over entry");

				return NULL;
			}

		} while (brute && bruteCount < bruteMax && !found);

		if (!found)
			MAC_RK_LOG("failed to find %s", prefix);

		return proc ? NULL : res;
	}

	bool usingPrelinkedCache()
	{
		IORegistryEntry *root = IORegistryEntry::getRegistryRoot();
		
		if (root)
		{
			OSNumber *count = OSDynamicCast(OSNumber, root->getProperty("OSPrelinkKextCount"));

			if (count)
			{
				MAC_RK_LOG("OSPrelinkKextCount equals to %u", count->unsigned32BitValue());

				return count->unsigned32BitValue() > 0;
			} else
			{
				MAC_RK_LOG("missing OSPrelinkKextCount property!");
			}
		} else
		{
			MAC_RK_LOG("missing registry root!");
		}

		return false;
	}

	bool renameDevice(IORegistryEntry *entry, char *name, bool compat)
	{
		if (!entry || !name)
			return false;

		entry->setName(name);

		if (!compat)
			return true;

		OSData *compatibleProp = OSDynamicCast(OSData, entry->getProperty("compatible"));
		
		if (!compatibleProp)
			return true;

		uint32_t compatibleSz = compatibleProp->getLength();

		const char *compatibleStr = static_cast<const char *>(compatibleProp->getBytesNoCopy());

		MAC_RK_LOG("compatible property starts with %s and is %u bytes", compatibleStr ? compatibleStr : "(null)", compatibleSz);

		if (compatibleStr) {
			for (uint32_t i = 0; i < compatibleSz; i++) {
				if (!strcmp(&compatibleStr[i], name)) {
					MAC_RK_LOG("found %s in compatible, ignoring", name);
					return true;
				}

				i += strlen(&compatibleStr[i]);
			}

			uint32_t nameSize = static_cast<uint32_t>(strlen(name) + 1);

			uint32_t compatibleBufSz = compatibleSz + nameSize;

			uint8_t *compatibleBuf = new uint8_t[compatibleBufSz];

			if (compatibleBuf)
			{
				MAC_RK_LOG("fixing compatible to have %s", name);

				memcpy(&compatibleBuf[0], compatibleStr, compatibleSz);
				memcpy(&compatibleBuf[compatibleSz], name, nameSize);

				OSData *compatibleData = OSData::withBytes(compatibleBuf, compatibleBufSz);

				if (compatibleData)
				{
					entry->setProperty("compatible", compatibleData);
					compatibleData->release();

					return true;
				} else
				{
					MAC_RK_LOG("compatible property memory alloc failure %u for %s", compatibleBufSz, name);
				}
			} else
			{
				MAC_RK_LOG("compatible buffer memory alloc failure %u for %s", compatibleBufSz, name);
			}
		}

		return false;
	}

	void patchVtableEntry(OSObject *object, void *entry, uint32_t idx)
	{

	}

	void patchVtable(OSObject *object, void *vtable)
	{
		
	}

}