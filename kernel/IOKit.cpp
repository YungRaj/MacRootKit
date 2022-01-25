#include "IOKit.hpp"

namespace IOKit
{
	bool awaitPublishing(IORegistryEntry *obj)
	{
		size_t counter = 0;

		while(counter < 256)
		{
			if(obj->inPlane(gIOServicePlane))
				return true;

			++counter;

			IOSleep(20);
		}

		return false;
	}

	uint32_t readPCIConfigValue(IORegistryEntry *service, uint32_t reg, uint32_t space, uint32_t size)
	{
		if(!awaitPublishing(service))
			return 0xffffffff;

		//if(space == 0)
			// space = getMember(service, 0xA8);
	
		if(size != 0)
		{
			switch(size)
			{
				case 8:
					break;
				case 16:
					break;
				default:
					break;
			}
		}

		switch(reg)
		{
			case kIOPCIConfigVendorID:
			case kIOPCIConfigDeviceID:
			case kIOPCIConfigCommand:
			case kIOPCIConfigStatus:
			case kIOPCIConfigRevisionID:
			case kIOPCIConfigClassCode:
			case kIOPCIConfigCacheLineSize:
			case kIOPCIConfigLatencyTimer:
			case kIOPCIConfigHeaderType:
			case kIOPCIConfigBIST:
			case kIOPCIConfigBaseAddress0:
			case kIOPCIConfigBaseAddress1:
			case kIOPCIConfigBaseAddress2:
			case kIOPCIConfigBaseAddress3:
			case kIOPCIConfigBaseAddress4:
			case kIOPCIConfigBaseAddress5:
			case kIOPCIConfigCardBusCISPPtr:
			case kIOPCIConfigSubSystemVendorID:
			case kIOPCIConfigSubSystemID:
			case kIOPCIConfigExpansionROMBase:
			case kIOPCIConfigCapabilitiesPtr:
			case kIOPCIConfigInterruptLine:
			case kIOPCIConfigInterruptPin:
			case kIOPCIConfigMinimumGrant:
			case kIOPCIConfigMaximumLatency:
			default:
				break;
		}

		return 0;
	}

	void getDeviceAddress(IORegistryEntry *service, uint8_t &bus, uint8_t &device, uint8_t &function)
	{

	}

	IORegistryEntry* findEntryByPrefix(const char *path, const char *prefix, const IORegistryPlane *plane, bool (*proc)(void*, IORegistryEntry*), bool brute, void *user)
	{
		auto entry = IORegistryEntry::fromPath(path, plane);

		if(entry)
		{
			auto res = findEntryByPrefix(entry, prefix, plane, proc, brute, user);

			entry->release();

			return res;
		}

		return NULL;
	}

	IORegistryEntry* findEntryByPrefix(IORegistryEntry *entry, const char *prefix, const IORegistryPlane *plane, bool (*proc)(void*, IORegistryEntry*), bool brute, void *user)
	{
		bool found = false;

		IORegistryEntry *res = NULL;

		size_t bruteCount = 0;

		static constexpr size_t bruteMax = 40000000;

		do
		{
			bruteCount++;

			auto iterator = entry->getChildIterator(plane);

			if(iterator)
			{
				size_t len = strlen(prefix);

				while((res = OSDynamicCast(IORegistryEntry, iterator->getNextObject())) != nullptr)
				{
					const char *resname = res->getName();

					if(resname && !strncmp(prefix, resname, len))
					{
						found = proc ? proc(user, res) : true;

						if(found)
						{
							if(!proc)
								break;
						}
					}
				}

				iterator->release();
			}
		} while(brute && bruteCount < bruteMax && !found);

		//if(!found)
		// LOG("Did not find IORegistry entry");

		return proc ? nullptr : res;
	}

	void patchVtableEntry(OSObject *object, void *entry, uint32_t idx)
	{

	}

	void patchVtable(OSObject *object, void *vtable)
	{
		
	}

}