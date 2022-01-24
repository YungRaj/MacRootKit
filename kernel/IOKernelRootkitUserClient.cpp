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
	IOKernelRootKitService *service = this->kernel->getRootKitService();

	this->rootkitService = service;

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

IOReturn IOKernelRootKitUserClient::externalMethod(UInt32 selector, IOExternalMethodArguments *arguments, IOExternalMethodDispatch *dispatch, OSObject *target, void *reference)
{
	IOReturn result = kIOReturnSuccess;

	MAC_RK_LOG("MacRK::IOKernelRootKitUserClient::externalMethod() called!\n");

	switch(selector)
	{
		case kIOKernelRootKitHookKernelFunction:
			;

			if(arguments->scalarInputCount == 3)
			{
				KernelPatcher *patcher = rootkitService->getRootKit()->getKernelPatcher();

				IOMemoryDescriptor *descriptor;

				IOMemoryMap *map;

				bool success;

				mach_vm_address_t address = arguments->scalarInput[0];

				mach_vm_address_t hook = arguments->scalarInput[1];

				size_t hook_size = arguments->scalarInput[2];

				size_t code_size = hook_size % 0x1000 >= sizeof(uint64_t) ? (hook_size - (hook_size % 0x1000)) + 0x1000 : hook_size;

				if(hook)
				{
				}
			}

			break;
		case kIOKernelRootKitAddBreakpoint:
			;

			if(arguments->scalarInputCount == 3)
			{
				KernelPatcher *patcher = rootkitService->getRootKit()->getKernelPatcher();

				IOMemoryDescriptor *descriptor;

				IOMemoryMap *map;

				bool success;

				mach_vm_address_t breakpoint = arguments->scalarInput[0];

				mach_vm_address_t breakpoint_hook = arguments->scalarInput[1];

				size_t breakpoint_hook_size = arguments->scalarInput[2];

				size_t code_size = breakpoint_hook_size % 0x1000 > 0 ? (breakpoint_hook_size - (breakpoint_hook_size % 0x1000)) + 0x1000 : breakpoint_hook_size;
			
				mach_vm_address_t copyin;

				if(breakpoint_hook)
				{

				}
			}

			break;
		case kIOKernelRootKitKernelCall:
			;

			if(arguments->scalarInputCount > 1)
			{
				mach_vm_address_t func = arguments->scalarInput[0];

				size_t argCount = arguments->scalarInputCount - 1;

				uint64_t *args = new uint64_t[argCount];

				for(uint32_t i = 1; i < argCount + 1; i++)
				{
					args[i - 1] = arguments->scalarInput[i];
				}

				uint64_t ret = kernel->call(func, (uint64_t*) args, argCount);

				arguments->scalarOutput[0] = ret;

				delete [] args;
			}

			break;
		case kIOKernelRootKitGetKaslrSlide:
			;

			if(arguments)
			{
				if(arguments->scalarInputCount == 0)
				{
					uint64_t slide = Kernel::findKernelSlide();

					arguments->scalarOutput[0] = slide;
				}
			}

			break;
		case kIOKernelRootKitGetKernelBase:
			;

			if(arguments)
			{
				if(arguments->scalarInputCount == 0)
				{
					mach_vm_address_t base = kernel->getBase();

					arguments->scalarOutput[0] = base;
				}
			}

			break;
		case kIOKernelRootKitGetKernelSymbol:
			;

			if(arguments)
			{
				if(arguments->scalarInputCount == 2)
				{
					IOMemoryDescriptor *descriptor;

					IOMemoryMap *map;

					Symbol *symbol;

					mach_vm_address_t symaddr;

					uint8_t *buf = this->mapBufferFromClientTask(arguments->scalarInput[0], arguments->scalarInput[1], kIODirectionOutIn, &descriptor, &map);

					char *symname = reinterpret_cast<char*>(buf);

					symaddr = kernel->getSymbolAddressByName(symname);

					arguments->scalarOutput[0] = symaddr;

					if(!symaddr)
					{
						result = kIOReturnBadArgument;
					}

					if(map)
						map->release();

					if(descriptor)
						descriptor->release();
				}
			}

			break;
		case kIOKernelRootKitKernelRead:
			;

			if(arguments)
			{
				if(arguments->scalarInputCount == 3)
				{
					bool success;

					IOMemoryDescriptor *descriptor;

					IOMemoryMap *map;

					mach_vm_address_t address = (mach_vm_address_t) arguments->scalarInput[0];

					uint64_t data = arguments->scalarInput[1];

					size_t size = arguments->scalarInput[2];

					uint8_t *buf = this->mapBufferFromClientTask(data, size, kIODirectionOutIn, &descriptor, &map);

					if(address && buf)
					{
						success = kernel->read(address, (void*) buf, size);

						if(!success)
						{
							result = kIOReturnNoMemory;
						}
					} else
					{
						result = kIOReturnBadArgument;
					}
				
					if(map)
						map->release();

					if(descriptor)
						descriptor->release();
				}
			}


			break;
		case kIOKernelRootKitKernelReadUnsafe:
			break;
		case kIOKernelRootKitKernelWrite:
			;

			if(arguments)
			{
				if(arguments->scalarInputCount == 3)
				{
					bool success;

					IOMemoryDescriptor *descriptor;

					IOMemoryMap *map;

					mach_vm_address_t address = (mach_vm_address_t) arguments->scalarInput[0];

					uint64_t data = arguments->scalarInput[1];

					size_t size = arguments->scalarInput[2];

					uint8_t *buf = this->mapBufferFromClientTask(data, size, kIODirectionOutIn, &descriptor, &map);

					if(address && buf)
					{
						uint8_t *buf_copy = new uint8_t[size];

						descriptor->readBytes(0, (void*) buf_copy, (uint32_t) size);

						kernel->write(address, (void*) buf_copy, size);

						if(!success)
						{
							result = kIOReturnNoMemory;
						}

						delete [] buf_copy;

						if(map)
							map->release();

						if(descriptor)
							descriptor->release();
					}
				} else
				{
					result = kIOReturnBadArgument;
				}
			}


			break;
		case kIOKernelRootKitKernelWriteUnsafe:
			break;
		case kIOKernelRootKitKernelVmAllocate:
			;

			if(arguments)
			{
				if(arguments->scalarInputCount == 1)
				{
					mach_vm_address_t address;

					size_t size = arguments->scalarInput[0];

					address = kernel->vmAllocate(size);

					if(!address)
					{
						result = kIOReturnNoMemory;
					}

					arguments->scalarOutput[0] = address;
				}
			}

			break;
		case kIOKernelRootKitKernelVmDeallocate:
			;

			if(arguments)
			{
				if(arguments->scalarInputCount == 2)
				{
					mach_vm_address_t address = arguments->scalarInput[0];

					size_t size = arguments->scalarInput[1];

					kernel->vmDeallocate(address, size);

					arguments->scalarOutput[0] = address;
				}
			}

			break;
		case kIOKernelRootKitKernelVmProtect:
			;

			if(arguments)
			{
				if(arguments->scalarInputCount == 3)
				{
					bool success;

					mach_vm_address_t address = arguments->scalarInput[0];

					size_t size = arguments->scalarInput[1];

					vm_prot_t prot = (vm_prot_t) arguments->scalarInput[2];

					if(address)
					{
						success = kernel->vmProtect(address, size, prot);

						if(!success)
						{
							result = kIOReturnNoMemory;
						}
					} else
					{
						result = kIOReturnBadArgument;
					}
				}
			}

			break;
		case kIOKernelRootKitKernelVmRemap:
			;

			if(arguments)
			{
				if(arguments->scalarInputCount == 2)
				{
					mach_vm_address_t remapped;

					mach_vm_address_t address = arguments->scalarInput[0];

					size_t size = arguments->scalarInput[1];

					if(address)
					{
						remapped = reinterpret_cast<mach_vm_address_t>(kernel->vmRemap(address, size));

						if(!remapped)
						{
							result = kIOReturnNoMemory;
						}
					} else
					{
						result = kIOReturnBadArgument;
					}
				}
			}


			break;
		case kIOKernelRootKitKalloc:
			break;
		case kIOKernelRootKitPhysicalRead:
			;

			if(arguments)
			{
				if(arguments->scalarInputCount == 3)
				{
					bool success;

					IOMemoryDescriptor *descriptor;

					IOMemoryMap *map;

					uint64_t paddr = arguments->scalarInput[0];

					uint64_t data = arguments->scalarInput[1];

					size_t size = arguments->scalarInput[2];

					uint8_t *buf = this->mapBufferFromClientTask(data, size, kIODirectionOutIn, &descriptor, &map);

					success = kernel->physicalRead(paddr, (void*) buf, size);

					if(!success)
					{
						result = kIOReturnNoMemory;
					}

					if(map)
						map->release();

					if(descriptor)
						descriptor->release();
				}
			}

			break;
		case kIOKernelRootKitPhysicalWrite:
			;

			if(arguments)
			{
				if(arguments->scalarInputCount == 3)
				{
					bool success;

					IOMemoryDescriptor *descriptor;

					IOMemoryMap *map;

					uint64_t paddr = arguments->scalarInput[0];

					uint64_t data = arguments->scalarInput[1];

					size_t size = arguments->scalarInput[2];

					uint8_t *buf = this->mapBufferFromClientTask(data, size, kIODirectionOutIn, &descriptor, &map);

					success = kernel->physicalWrite(paddr, (void*) buf, size);

					if(!success)
					{
						result = kIOReturnNoMemory;
					}

					if(map)
						map->release();

					if(descriptor)
						descriptor->release();
				}
			}

			break;
		case kIOKernelRootKitKernelVirtualToPhysical:
			;

			if(arguments)
			{
				if(arguments->scalarInputCount == 1)
				{
					uint64_t physaddr;

					uint64_t vmaddr = (mach_vm_address_t) arguments->scalarInput[0];

					if(vmaddr)
					{
						physaddr = kernel->virtualToPhysical(vmaddr);

						if(!physaddr)
						{
							result = kIOReturnNoMemory;
						}
					} else
					{
						result = kIOReturnBadArgument;
					}
				}
			}

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
