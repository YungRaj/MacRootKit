#include "IOKernelRootKitUserClient.hpp"
#include "IOKernelRootKitService.hpp"

#include "MacRootKit.hpp"

#include "Kernel.hpp"
#include "KernelPatcher.hpp"

#include "Task.hpp"

#include "Segment.hpp"
#include "Section.hpp"

#include "Log.hpp"

#include <mach/vm_types.h>

extern "C"
{
	#include "kern.h"
}

OSDefineMetaClassAndStructors(IOKernelRootKitUserClient, IOUserClient)

IOKernelRootKitUserClient* IOKernelRootKitUserClient::rootKitUserClientWithKernel(Kernel *kernel, task_t owningTask, void *securityToken, UInt32 type)
{
	IOKernelRootKitUserClient *client;

	client = new IOKernelRootKitUserClient();

	if(client)
	{
		if(!client->initRootKitUserClientWithKernel(kernel, owningTask, securityToken, type))
		{
			client->release();

			return NULL;
		}
	}

	return client;
}

IOKernelRootKitUserClient* IOKernelRootKitUserClient::rootKitUserClientWithKernel(Kernel *kernel, task_t owningTask, void *securityToken, UInt32 type, OSDictionary *properties)
{
	IOKernelRootKitUserClient *client;

	client = new IOKernelRootKitUserClient();

	if(client)
	{
		if(!client->initRootKitUserClientWithKernel(kernel, owningTask, securityToken, type, properties))
		{
			client->release();

			return NULL;
		}
	}

	return client;
}

bool IOKernelRootKitUserClient::initRootKitUserClientWithKernel(Kernel *kernel, task_t owningTask, void *securityToken, UInt32 type)
{
	bool result = IOUserClient::initWithTask(owningTask, securityToken, type);

	if(!kernel)
		result = false;

	this->kernel = kernel;

	this->clientTask = owningTask;
	this->kernelTask = *(task_t*) kernel->getSymbolAddressByName("_kernel_task");

	return result;
}

bool IOKernelRootKitUserClient::initRootKitUserClientWithKernel(Kernel *kernel, task_t owningTask, void *securityToken, UInt32 type, OSDictionary *properties)
{
	bool result = IOUserClient::initWithTask(owningTask, securityToken, type, properties);

	if(!kernel)
		result = false;

	this->kernel = kernel;

	this->clientTask = owningTask;
	this->kernelTask = *(task_t*) kernel->getSymbolAddressByName("_kernel_task");

	return result;
}

bool IOKernelRootKitUserClient::start(IOService *provider)
{
	// IOKernelRootKitService *service = this->kernel->getRootKitService();

	// this->rootkitService = service;

	return IOUserClient::start(provider);
}

void IOKernelRootKitUserClient::stop(IOService *provider)
{
	return IOUserClient::stop(provider);
}

IOReturn IOKernelRootKitUserClient::clientClose()
{
	return kIOReturnSuccess;
}

IOReturn IOKernelRootKitUserClient::clientDied()
{
	IOReturn result = clientClose();

	return result;
}

void IOKernelRootKitUserClient::free()
{
}

IOExternalMethod* IOKernelRootKitUserClient::getExternalMethodForIndex(UInt32 index)
{
	return NULL;
}

IOExternalTrap*  IOKernelRootKitUserClient::getExternalTrapForIndex(UInt32 index)
{
	return NULL;
}

uint8_t* IOKernelRootKitUserClient::mapBufferFromClientTask(mach_vm_address_t uaddr, size_t size, IOOptionBits options, IOMemoryDescriptor **desc, IOMemoryMap **mapping)
{
	uint8_t *buffer;

	IOReturn ret;

	IOMemoryDescriptor *descriptor;

	IOMemoryMap *map;

	descriptor = IOMemoryDescriptor::withAddressRange(uaddr, size, options, this->getClientTask());

	if(!descriptor)
	{
		goto fail;
	}

	ret = descriptor->prepare(options);

	if(ret != kIOReturnSuccess)
	{
		goto fail;
	}

	map = descriptor->map();

	if(!map)
	{
		goto fail;
	}

	buffer = reinterpret_cast<uint8_t*>(map->getVirtualAddress());

	if(!buffer)
	{
		goto fail;
	}

	*desc = descriptor;
	*mapping = map;

	return buffer;

fail:
	*desc = NULL;
	*mapping = NULL;

	return NULL;
}

IOReturn externalMethod(UInt32 selector, IOExternalMethodArguments *arguments, IOExternalMethodDispatch *dispatch, OSObject *target, void *reference)
{
	IOReturn result = kIOReturnSuccess;

	switch(selector)
	{
		case kIOKernelRootKitHookKernelFunction:
			break;
		case kIOKernelRootKitAddBreakpoint:
			break;
		case kIOKernelRootKitKernelCall:
			break;
		case kIOKernelRootKitGetKaslrSlide:
			break;
		case kIOKernelRootKitGetKernelBase:
			break;
		case kIOKernelRootKitGetKernelSymbol:
			break;
		case kIOKernelRootKitKernelRead:
			break;
		case kIOKernelRootKitKernelReadUnsafe:
			break;
		case kIOKernelRootKitKernelWrite:
			break;
		case kIOKernelRootKitKernelWriteUnsafe:
			break;
		case kIOKernelRootKitKernelVmAllocate:
			break;
		case kIOKernelRootKitKernelVmDeallocate:
			break;
		case kIOKernelRootKitKernelVmProtect:
			break;
		case kIOKernelRootKitKernelVmRemap:
			break;
		case kIOKernelRootKitKalloc:
			break;
		case kIOKernelRootKitPhysicalRead:
			break;
		case kIOKernelRootKitPhysicalWrite:
			break;
		case kIOKernelRootKitKernelVirtualToPhysical:
			break;
		case kIOKernelRootKitTaskForPid:
			break;
		case kIOKernelRootKitMachVmRead:
			break;
		case kIOKernelRootKitMachVmWrite:
			break;
		case kIOKernelRootKitMachVmAllocate:
			break;
		case kIOKernelRootKitMachVmProtect:
			break;
		case kIOKernelRootKitMachVmCall:
			break;
		case kIOKernelRootKitVirtualToPhysical:
			break;
		case kIOKernelRootKitMmap:
			break;
		case kIOKernelRootKitMachMsgSend:
			break;
		case kIOKernelRootKitCopyIn:
			break;
		case kIOKernelRootKitCopyOut:
			break;
		case kIOKernelRootKitCreateSharedMemory:
			break;
		case kIOKernelRootKitMapSharedMemory:
			break;

		default:
			result = kIOReturnUnsupported;

			break;
	}

	return result;
}
