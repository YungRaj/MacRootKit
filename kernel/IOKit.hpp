#ifndef __IOKIT_HPP_
#define __IOKIT_HPP_

#include <IOKit/IOLib.h>

#include <IOKit/IOService.h>
#include <IOKit/IOUserClient.h>
#include <IOKit/IORegistryEntry.h>
#include <IOKit/IODeviceTreeSupport.h>

#include <libkern/c++/OSSerialize.h>

#include <mach/mach_types.h>

namespace IOKit
{
	enum PCIRegister : uint8_t
	{
		kIOPCIConfigVendorID 				= 0x00,
		kIOPCIConfigDeviceID 				= 0x02,
		kIOPCIConfigCommand 				= 0x04,
		kIOPCIConfigStatus 					= 0x06,
		kIOPCIConfigRevisionID 				= 0x08,
		kIOPCIConfigClassCode 				= 0x09,
		kIOPCIConfigCacheLineSize 			= 0x0C,
		kIOPCIConfigLatencyTimer 			= 0x0D,
		kIOPCIConfigHeaderType 				= 0x0E,
		kIOPCIConfigBIST 					= 0xF,
		kIOPCIConfigBaseAddress0 			= 0x10,
		kIOPCIConfigBaseAddress1 			= 0x14,
		kIOPCIConfigBaseAddress2 			= 0x18,
		kIOPCIConfigBaseAddress3 			= 0x1C,
		kIOPCIConfigBaseAddress4 			= 0x20,
		kIOPCIConfigBaseAddress5 			= 0x24,
		kIOPCIConfigCardBusCISPtr 			= 0x28,
		kIOPCIConfigSubSystemVendorID 		= 0x2C,
		kIOPCIConfigSubSystemID 			= 0x2E,
		kIOPCIConfigExpansionROMBase 		= 0x30,
		kIOPCIConfigCapabilitiesPtr 		= 0x34,
		kIOPCIConfigInterruptLine 			= 0x3C,
		kIOPCIConfigInterruptPin 			= 0x3D,
		kIOPCIConfigMinimumGrant			= 0x3E,
		kIOPCIConfigMaximumLatency			= 0x3F,
		kIOPCIConfigGraphicsControl 		= 0x50
	};

	struct PCIConfigOffset
	{
		enum : size_t
		{
			ConfigRead32 		= 0x10A,
			ConfigWrite32 		= 0x10B,
			ConfigRead16 		= 0x10C,
			ConfigWrite16 		= 0x10D,
			ConfigRead8 		= 0x10E,
			ConfigWrite8 		= 0x10F,
			GetBusNumber 		= 0x11D,
			GetDeviceNumber 	= 0x11E,
			GetFunctionNumber 	= 0x11F
		};
	};
	
	using t_PCIConfigRead8 					= uint32_t (*)(IORegistryEntry *service, uint32_t space, uint8_t offset);
	using t_PCIConfigRead16 				= uint16_t (*)(IORegistryEntry *service, uint32_t space, uint8_t offset);
	using t_PCIConfigRead32 				= uint8_t (*)(IORegistryEntry *service, uint32_t space, uint8_t offset);

	using t_PCIConfigWrite8 				= void (*)(IORegistryEntry *service, uint32_t space, uint8_t offset, uint32_t data);
	using t_PCIConfigWrite16 				= void (*)(IORegistryEntry *service, uint32_t space, uint8_t offset, uint16_t data);
	using t_PCIConfigWrite32 				= void (*)(IORegistryEntry *service, uint32_t space, uint8_t offset, uint8_t data);

	using t_PCIConfigGetBusNumber 			= uint8_t (*)(IORegistryEntry *service);
	using t_PCIConfigGetDeviceNumber 		= uint8_t (*)(IORegistryEntry *service);
	using t_PCIConfigGetFunctionNumber 		= uint8_t (*)(IORegistryEntry *service);

	bool awaitPublishing(IORegistryEntry *obj);

	uint32_t readPCIConfigValue(IORegistryEntry *service, uint32_t reg, uint32_t space = 0, uint32_t size = 0);

	void getDeviceAddress(IORegistryEntry *service, uint8_t &bus, uint8_t &device, uint8_t &function);

	IORegistryEntry* findEntryByPrefix(const char *path, const char *prefix, const IORegistryPlane *plane, bool (*proc)(void*, IORegistryEntry*) = nullptr, bool brute = false, void *user = nullptr);

	IORegistryEntry* findEntryByPrefix(IORegistryEntry *entry, const char *prefix, const IORegistryPlane *plane, bool (*proc)(void*, IORegistryEntry*) = nullptr, bool brute = false, void *user = nullptr);

	template<typename T>
	bool getOSDataValue(const OSObject *obj, const char *name, T &value);

	OSSerialize* getProperty(IORegistryEntry *entry, const char *property);

	void patchVtableEntry(OSObject *object, void *entry, uint32_t idx);

	void patchVtable(OSObject *object, void *vtable);

};

#endif