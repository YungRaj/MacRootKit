#include "IOKernelRootKitService.hpp"
#include "IOKernelRootKitUserClient.hpp"

#include "MacRootKit.hpp"

#include "Kernel.hpp"

#include "Kext.hpp"
#include "KextMachO.hpp"

#include "Log.hpp"

#define super IOService

OSDefineMetaClassAndStructors(IOKernelRootKitService, IOService)

bool IOKernelRootKitService::init(OSDictionary *properties)
{
	userClients = OSSet::withCapacity(1);

	if(!userClients)
		return false;

	return super::init(properties);
}

void IOKernelRootKitService::free()
{
	IOService::free();
}

bool IOKernelRootKitService::start(IOService *provider)
{
	kern_return_t ret = kIOReturnSuccess;

	mach_vm_address_t kernel_base = Kernel::findKernelBase();

	MAC_RK_LOG("MacRK::START KERNEL EXTENSION!\n");

	// off_t kernel_slide = Kernel::findKernelSlide();

	// this->kernel = new Kernel(kernel_base, kernel_slide);

	// this->kernel->setRootKitService(this);

	// this->tfp0 = this->kernel->getKernelTaskPort();

	// ret = mac_rootkit_start(this, this->kernel, &this->rootkitKext);

	if(ret == kIOReturnSuccess)
	{
		// this->rootkit = mac_rootkit_get_rootkit();

		registerService();
	}

	return ret == kIOReturnSuccess && IOService::start(provider);
}

void IOKernelRootKitService::stop(IOService *provider)
{
	kern_return_t ret;

	ret = mac_rootkit_stop(this, this->kernel, &this->rootkitKext);

	if(ret != KERN_SUCCESS)
	{
		return;
	}

	if(userClients)
	{
		this->detachUserClients();
	}

	IOService::stop(provider);
}

IOService* IOKernelRootKitService::probe(IOService *provider, SInt32 *score)
{
	return IOService::probe(provider, score);
}

void IOKernelRootKitService::clientClosed(IOKernelRootKitUserClient *client)
{
	if(client)
	{
		this->removeUserClient(client);
	}
}

IOReturn IOKernelRootKitService::createUserClient(task_t task, void *securityID, UInt32 type, IOKernelRootKitUserClient **client)
{
	IOReturn result = kIOReturnSuccess;

	IOKernelRootKitUserClient *userClient;

	userClient = IOKernelRootKitUserClient::rootKitUserClientWithKernel(this->kernel, task, securityID, type);

	if(userClient)
		*client = userClient;
	else
		result = kIOReturnNoMemory;

	return result;
}

IOReturn IOKernelRootKitService::createUserClient(task_t task, void *securityID, UInt32 type, OSDictionary *properties, IOKernelRootKitUserClient **client)
{
	IOReturn result = kIOReturnSuccess;

	IOKernelRootKitUserClient *userClient;

	userClient = IOKernelRootKitUserClient::rootKitUserClientWithKernel(this->kernel, task, securityID, type, properties);

	if(userClient)
		*client = userClient;
	else
		result = kIOReturnNoMemory;

	return result;
}

IOReturn IOKernelRootKitService::newUserClient(task_t task, void *securityID, UInt32 type, OSDictionary *properties, IOUserClient **client)
{
	IOReturn result;

	IOKernelRootKitUserClient *userClient;

	if(!isInactive())
	{
		result = this->createUserClient(task, securityID, type, properties, &userClient);

		if((result == kIOReturnSuccess) && (userClient != NULL))
		{
			if(!reinterpret_cast<IOService*>(userClient)->attach(this))
			{
				result = kIOReturnError;
			} else if(!userClient->start(this))
			{
				reinterpret_cast<IOService*>(userClient)->detach(this);

				result = kIOReturnError;
			} else
			{
				userClients->setObject((OSObject*) userClient);
			}

			*client = reinterpret_cast<IOUserClient*>(userClient);
		}
	} else
	{
		result = kIOReturnNoDevice;
	}

	return result;
}

IOReturn IOKernelRootKitService::newUserClient(task_t task, void *securityID, UInt32 type, IOUserClient **client)
{
	IOReturn result;

	IOKernelRootKitUserClient *userClient;

	if(!isInactive())
	{
		result = this->createUserClient(task, securityID, type, &userClient);

		if((result == kIOReturnSuccess) && (userClient != NULL))
		{
			if(!reinterpret_cast<IOService*>(userClient)->attach(this))
			{
				result = kIOReturnError;
			} else if(!userClient->start(this))
			{
				reinterpret_cast<IOService*>(userClient)->detach(this);

				result = kIOReturnError;
			} else
			{
				userClients->setObject((OSObject*) userClient);
			}

			*client = reinterpret_cast<IOUserClient*>(userClient);
		}
	} else
	{
		result = kIOReturnNoDevice;
	}

	return result;
}

IOReturn IOKernelRootKitService::addUserClient(IOKernelRootKitUserClient *client)
{
	IOReturn result = kIOReturnSuccess;

	if(!isInactive())
	{
		if(!reinterpret_cast<IOService*>(client)->attach(this))
		{
			result = kIOReturnError;
		} else if(!client->start(this))
		{
			reinterpret_cast<IOService*>(client)->detach(this);

			result = kIOReturnError;
		} else
		{
			userClients->setObject((OSObject*) client);
		}
	} else
	{
		result = kIOReturnNoDevice;
	}

	return result;
}

IOReturn IOKernelRootKitService::removeUserClient(IOKernelRootKitUserClient *client)
{
	IOService *userClient = dynamic_cast<IOService*>(client);

	userClient->retain();

	userClients->removeObject((OSObject*) userClient);

	if(!isInactive())
	{
		userClient->terminate();
	}

	userClient->release();

	return kIOReturnSuccess;
}

IOReturn IOKernelRootKitService::detachUserClients()
{
	IOReturn result = kIOReturnSuccess;

	if(!isInactive())
	{
		OSIterator *iterator;

		iterator = OSCollectionIterator::withCollection(userClients);

		if(iterator)
		{
			IOKernelRootKitUserClient *client;

			while((client = (IOKernelRootKitUserClient*) iterator->getNextObject()))
			{
				reinterpret_cast<IOService*>(client)->terminate();
			}

			iterator->release();
		}
	}

	userClients->flushCollection();

	return result;
}
