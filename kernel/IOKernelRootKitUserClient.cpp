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

IOKernelRootKitUserClient* IOKernelRootKitUserClient::rootKitUserClientWithKernel(xnu::Kernel *kernel, task_t owningTask, void *securityToken, UInt32 type)
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

IOKernelRootKitUserClient* IOKernelRootKitUserClient::rootKitUserClientWithKernel(xnu::Kernel *kernel, task_t owningTask, void *securityToken, UInt32 type, OSDictionary *properties)
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

bool IOKernelRootKitUserClient::initRootKitUserClientWithKernel(xnu::Kernel *kernel, task_t owningTask, void *securityToken, UInt32 type)
{
	bool result = IOUserClient::initWithTask(owningTask, securityToken, type);

	if(!kernel)
		result = false;

	this->kernel = kernel;

	this->clientTask = owningTask;
	this->kernelTask = *(task_t*) kernel->getSymbolAddressByName("_kernel_task");

	return result;
}

bool IOKernelRootKitUserClient::initRootKitUserClientWithKernel(xnu::Kernel *kernel, task_t owningTask, void *securityToken, UInt32 type, OSDictionary *properties)
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

		case kIOKernelRootKitGetKextSymbol:
			;

			if(arguments)
			{
				if(arguments->scalarInputCount == 4)
				{
					IOMemoryDescriptor *descriptor;

					IOMemoryMap *map;

					Symbol *symbol;

					mach_vm_address_t symaddr;

					uint8_t *buf1 = this->mapBufferFromClientTask(arguments->scalarInput[0], arguments->scalarInput[1], kIODirectionOutIn, &descriptor, &map);

					uint8_t *buf2= this->mapBufferFromClientTask(arguments->scalarInput[2], arguments->scalarInput[3], kIODirectionOutIn, &descriptor, &map);

					char *kextidentifier= reinterpret_cast<char*>(buf1);

					xnu::Kext *kext = this->getRootKitService()->getRootKit()->getKextByIdentifier(kextidentifier);

					if(kext)
					{
						char *kextsymname = reinterpret_cast<char*>(buf2);

						symaddr = kext->getSymbolAddressByName(kextsymname);

						arguments->scalarOutput[0] = symaddr;

						if(!symaddr)
						{
							result = kIOReturnBadArgument;
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

		case kIOKernelRootKitGetTaskForPid:
			;

			if(arguments)
			{
				if(arguments->scalarInputCount == 1)
				{
					int pid = (int) arguments->scalarInput[0];

					mach_vm_address_t task = reinterpret_cast<mach_vm_address_t>(Task::findTaskByPid(this->kernel, pid));

					if(!task)
					{
						result = kIOReturnError;
					}

					arguments->scalarOutput[0] = task;
				}
			}

			break;

		case kIOKernelRootKitGetProcForPid:
			;

			if(arguments)
			{
				if(arguments->scalarInputCount == 1)
				{
					int pid = (int) arguments->scalarInput[0];

					mach_vm_address_t proc = reinterpret_cast<mach_vm_address_t>(Task::findProcByPid(this->kernel, pid));

					if(!proc)
					{
						result = kIOReturnError;
					}
					
					arguments->scalarOutput[0] = proc;
				}
			}

			break;

		case kIOKernelRootKitGetTaskByName:
			;

			if(arguments)
			{
				if(arguments->scalarInputCount == 2)
				{
					bool success;

					IOMemoryDescriptor *descriptor;

					IOMemoryMap *map;

					char *name = reinterpret_cast<char*>(this->mapBufferFromClientTask(arguments->scalarInput[0], arguments->scalarInput[1], kIODirectionOutIn, &descriptor, &map));

					MAC_RK_LOG("MacRK::finding task with name = 0x%llx\n", (uint64_t)(name));

					mach_vm_address_t task = reinterpret_cast<mach_vm_address_t>(Task::findTaskByName(this->kernel, name));

					if(!task)
					{
						result = kIOReturnError;
					}
				
					if(map)
						map->release();

					if(descriptor)
						descriptor->release();

					arguments->scalarOutput[0] = task;
				}
			}

			break;

		case kIOKernelRootKitGetProcByName:
			;

			if(arguments)
			{
				if(arguments->scalarInputCount == 2)
				{
					bool success;

					IOMemoryDescriptor *descriptor;

					IOMemoryMap *map;

					char *name = reinterpret_cast<char*>(this->mapBufferFromClientTask(arguments->scalarInput[0], arguments->scalarInput[1], kIODirectionOutIn, &descriptor, &map));

					MAC_RK_LOG("MacRK::finding proc with 0x%llx\n", (uint64_t)(name));

				 	mach_vm_address_t proc = reinterpret_cast<mach_vm_address_t>(Task::findProcByName(this->kernel, name));

					if(!proc)
					{
						result = kIOReturnError;
					}
				
					if(map)
						map->release();

					if(descriptor)
						descriptor->release();

					arguments->scalarOutput[0] = proc;
				}
			}

			break;

		case kIOKernelRootKitMachVmRead:
			;

			if(arguments)
			{
				if(arguments->scalarInputCount == 4)
				{
					/*
					kern_return_t kr;

					IOMemoryDescriptor *descriptor;

					IOMemoryMap *mapping;

					mach_port_name_t task_port = (mach_port_name_t) arguments->scalarInput[0];

					mach_vm_address_t address = (mach_vm_address_t) arguments->scalarInput[1];

					if(task_port == MACH_PORT_NULL)
						break;

					uint64_t data = (uint64_t) arguments->scalarInput[2];

					size_t size = (size_t) arguments->scalarInput[3];

					vm_map_t (*_convert_port_to_map_read) (ipc_port_t port);

					typedef vm_map_t (*convert_port_to_map_read) (ipc_port_t port);

					ipc_space_t (*_get_task_ipcspace) (task_t t);

					typedef ipc_space_t (*get_task_ipcspace)(task_t);

					ipc_entry_t (*_ipc_entry_lookup) (ipc_space_t space, mach_port_name_t name);

					typedef ipc_entry_t (*ipc_entry_lookup) (ipc_space_t space, mach_port_name_t name);

					_convert_port_to_map_read = reinterpret_cast<convert_port_to_map_read> (this->kernel->getSymbolAddressByName("_convert_port_to_map_read"));

					_get_task_ipcspace = reinterpret_cast<get_task_ipcspace> (this->kernel->getSymbolAddressByName("_get_task_ipcspace"));

					_ipc_entry_lookup = reinterpret_cast<ipc_entry_lookup> (this->kernel->getSymbolAddressByName("_ipc_entry_lookup"));

					ipc_space_t space = _get_task_ipcspace(getClientTask());

					ipc_entry_t entry = _ipc_entry_lookup(space, task_port);

					ipc_port_t port = (ipc_port_t) entry->ie_object;
					
					vm_map_t map = _convert_port_to_map_read(port);

					size_t data_size;

					uint8_t *buf = this->getUserSpaceBuffer(data, size, kIODirectionOutIn, &descriptor, &mapping);

					if(address != 0)
					{
						kern_return_t (*_vm_read)(vm_map_t, vm_address_t, vm_offset_t, vm_address_t, vm_size_t*);

						_vm_read = (kern_return_t(*)(vm_map_t, vm_address_t, vm_offset_t, vm_address_t, vm_size_t*)) vm_read_;

						kr = _vm_read(map, address, size, (vm_address_t) buf, &data_size);

						if(kr != KERN_SUCCESS)
						{
							result = kIOReturnNoMemory;
						}

					} else
					{
						result = kIOReturnBadArgument;
					}

					if(mapping)
						mapping->release();

					if(descriptor)
						descriptor->release();
					*/
				}
			}

			break;
		case kIOKernelRootKitMachVmWrite:
			;

			if(arguments)
			{
				if(arguments->scalarInputCount == 4)
				{
					/*
					kern_return_t kr;

					mach_port_name_t task_port = (mach_port_name_t) arguments->scalarInput[0];

					mach_vm_address_t address = (mach_vm_address_t) arguments->scalarInput[1];

					if(task_port == MACH_PORT_NULL)
						break;

					uint64_t data = (uint64_t) arguments->scalarInput[2];

					size_t size = (size_t) arguments->scalarInput[3];

					vm_map_t (*_convert_port_to_map_read) (ipc_port_t port);

					typedef vm_map_t (*convert_port_to_map_read) (ipc_port_t port);

					ipc_space_t (*_get_task_ipcspace) (task_t t);

					typedef ipc_space_t (*get_task_ipcspace)(task_t);

					ipc_entry_t (*_ipc_entry_lookup) (ipc_space_t space, mach_port_name_t name);

					typedef ipc_entry_t (*ipc_entry_lookup) (ipc_space_t space, mach_port_name_t name);

					_convert_port_to_map_read = reinterpret_cast<convert_port_to_map_read> (this->kernel->getSymbolAddressByName("_convert_port_to_map_read"));

					_get_task_ipcspace = reinterpret_cast<get_task_ipcspace> (this->kernel->getSymbolAddressByName("_get_task_ipcspace"));

					_ipc_entry_lookup = reinterpret_cast<ipc_entry_lookup> (this->kernel->getSymbolAddressByName("_ipc_entry_lookup"));

					ipc_space_t space = _get_task_ipcspace(getClientTask());

					ipc_entry_t entry = _ipc_entry_lookup(space, task_port);

					ipc_port_t port = (ipc_port_t) entry->ie_object;

					vm_map_t dst_map = _convert_port_to_map_read(port);

					typedef vm_map_t (*get_task_map) (task_t task);
					vm_map_t (*_get_task_map)(task_t);

					_get_task_map = reinterpret_cast<get_task_map> (this->kernel->getSymbolAddressByName("_get_task_map"));

					vm_map_t src_map = _get_task_map(getClientTask());

					struct vm_map_copy
					{
						int                     type;
						vm_object_offset_t      offset;
						vm_map_size_t           size;
						void                    *kdata;
					};

					typedef struct vm_map_copy      *vm_map_copy_t;

					if(address != 0)
					{
						vm_map_copy_t copy;

						typedef kern_return_t (*vm_map_copyin)(vm_map_t, vm_map_address_t, vm_map_size_t, boolean_t, vm_map_copy_t*);
						
						kern_return_t (*_vm_map_copyin)(vm_map_t, vm_map_address_t, vm_map_size_t, boolean_t, vm_map_copy_t*);

						_vm_map_copyin = reinterpret_cast<vm_map_copyin>(this->kernel->getSymbolAddressByName("_vm_map_copyin"));
						
						kr = _vm_map_copyin(src_map, (vm_address_t) data, (vm_map_size_t) size, FALSE, &copy);

						typedef kern_return_t (*vm_map_copy_overwrite)(vm_map_t, vm_map_offset_t, vm_map_copy_t, vm_map_size_t, boolean_t);

						kern_return_t (*_vm_map_copy_overwrite)(vm_map_t, vm_map_offset_t, vm_map_copy_t, vm_map_size_t, boolean_t);

						_vm_map_copy_overwrite = reinterpret_cast<vm_map_copy_overwrite>(this->kernel->getSymbolAddressByName("_vm_map_copy_overwrite"));

						kr = _vm_map_copy_overwrite(dst_map, address, copy, size, FALSE);

						if(kr != KERN_SUCCESS)
						{
							result = kIOReturnNoMemory;
						}
					} else
					{
						result = kIOReturnBadArgument;
					}
					*/
				}
			}
			
			break;
		case kIOKernelRootKitMachVmAllocate:
			break;
		case kIOKernelRootKitMachVmDeallocate:
			;

			if(arguments)
			{
				if(arguments->scalarInputCount == 2)
				{
					/*
					kern_return_t kr;

					mach_port_name_t task_port = (mach_port_name_t) arguments->scalarInput[0];

					mach_vm_address_t address = (mach_vm_address_t) arguments->scalarInput[1];

					size_t size = (size_t) arguments->scalarInput[2];

					if(task_port == MACH_PORT_NULL)
						break;

					vm_map_t (*_convert_port_to_map_read) (ipc_port_t port);

					typedef vm_map_t (*convert_port_to_map_read) (ipc_port_t port);

					ipc_space_t (*_get_task_ipcspace) (task_t t);

					typedef ipc_space_t (*get_task_ipcspace)(task_t);

					ipc_entry_t (*_ipc_entry_lookup) (ipc_space_t space, mach_port_name_t name);

					typedef ipc_entry_t (*ipc_entry_lookup) (ipc_space_t space, mach_port_name_t name);

					_convert_port_to_map_read = reinterpret_cast<convert_port_to_map_read> (this->kernel->getSymbolAddressByName("_convert_port_to_map_read"));

					_get_task_ipcspace = reinterpret_cast<get_task_ipcspace> (this->kernel->getSymbolAddressByName("_get_task_ipcspace"));

					_ipc_entry_lookup = reinterpret_cast<ipc_entry_lookup> (this->kernel->getSymbolAddressByName("_ipc_entry_lookup"));

					ipc_space_t space = _get_task_ipcspace(getClientTask());

					ipc_entry_t entry = _ipc_entry_lookup(space, task_port);

					ipc_port_t port = (ipc_port_t) entry->ie_object;

					vm_map_t map = _convert_port_to_map_read(port);

					kr = vm_deallocate(map, address, size);

					if(kr != KERN_SUCCESS)
					{
						result = kIOReturnNoMemory;
					}
					*/
				}
			}
		case kIOKernelRootKitMachVmProtect:
			;

			if(arguments)
			{
				if(arguments->scalarInputCount == 4)
				{
					/*
					kern_return_t kr;

					mach_port_name_t task_port = (mach_port_name_t) arguments->scalarInput[0];

					mach_vm_address_t address = (mach_vm_address_t) arguments->scalarInput[1];

					if(task_port == MACH_PORT_NULL)
						break;

					size_t size = (size_t) arguments->scalarInput[2];

					vm_prot_t prot = (vm_prot_t) arguments->scalarInput[3];

					vm_map_t (*_convert_port_to_map_read) (ipc_port_t port);

					typedef vm_map_t (*convert_port_to_map_read) (ipc_port_t port);

					ipc_space_t (*_get_task_ipcspace) (task_t t);

					typedef ipc_space_t (*get_task_ipcspace)(task_t);

					ipc_entry_t (*_ipc_entry_lookup) (ipc_space_t space, mach_port_name_t name);

					typedef ipc_entry_t (*ipc_entry_lookup) (ipc_space_t space, mach_port_name_t name);

					_convert_port_to_map_read = reinterpret_cast<convert_port_to_map_read> (this->kernel->getSymbolAddressByName("_convert_port_to_map_read"));

					_get_task_ipcspace = reinterpret_cast<get_task_ipcspace> (this->kernel->getSymbolAddressByName("_get_task_ipcspace"));

					_ipc_entry_lookup = reinterpret_cast<ipc_entry_lookup> (this->kernel->getSymbolAddressByName("_ipc_entry_lookup"));

					ipc_space_t space = _get_task_ipcspace(getClientTask());

					ipc_entry_t entry = _ipc_entry_lookup(space, task_port);

					ipc_port_t port = (ipc_port_t) entry->ie_object;

					vm_map_t map = _convert_port_to_map_read(port);

					if(address != 0)
					{
						kern_return_t(*_vm_protect)(vm_map_t, vm_address_t, vm_size_t, boolean_t, vm_prot_t);

						_vm_protect = (kern_return_t(*)(vm_map_t, vm_address_t, vm_size_t, boolean_t, vm_prot_t)) vm_protect_;

						kr = _vm_protect(map, address, size, true, prot);

						if(kr != KERN_SUCCESS)
						{
							result = kIOReturnNoMemory;
						}
					} else
					{
						result = kIOReturnBadArgument;
					}
					*/
				}
			}

			break;
		case kIOKernelRootKitMachVmCall:
			break;
		case kIOKernelRootKitVirtualToPhysical:
			;

			if(arguments)
			{
				if(arguments->scalarInputCount == 2)
				{
					uint64_t paddr;

					mach_port_t task_port = (mach_port_t) arguments->scalarInput[0];

					mach_vm_address_t vaddr = (mach_vm_address_t) arguments->scalarInput[1];

					/*
					if(paddr)
					{
						result = kIOReturnNoMemory;
					}
					*/

					result = kIOReturnUnsupported;
				}
			}

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
			result = IOUserClient::externalMethod(selector, arguments, NULL, target, reference);

			break;
	}

	return result;
}
