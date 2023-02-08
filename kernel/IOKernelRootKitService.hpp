#ifndef __IOKERNELRootKitSERVICE_HPP_
#define __IOKERNELRootKitSERVICE_HPP_

#include <IOKit/IOLib.h>
#include <IOKit/IOService.h>

#include "MacRootKit.hpp"

#include "Kernel.hpp"
#include "Kext.hpp"

namespace mrk
{
	class MacRootKit;
};

class IOKernelRootKitService;
class IOKernelRootKitUserClient;

extern kern_return_t mac_rootkit_start(IOKernelRootKitService *service, xnu::Kernel *kernel, xnu::Kext **kext);
extern kern_return_t mac_rootkit_stop(IOKernelRootKitService * service, xnu::Kernel *kernel, xnu::Kext **kext);

extern mrk::MacRootKit* mac_rootkit_get_rootkit();

class IOKernelRootKitService : public IOService
{
	OSDeclareDefaultStructors(IOKernelRootKitService)

	public:
		virtual bool init(OSDictionary *properties) override;

		virtual void free() override;

		virtual bool start(IOService *provider) override;
		virtual void stop(IOService *provider) override;

		virtual IOService* probe(IOService *provider, SInt32 *score) override;

		virtual void clientClosed(IOUserClient *client);

		mrk::MacRootKit* getRootKit() { return rootkit; }

		xnu::Kernel* getKernel() { return kernel; }

		xnu::Kext* getKext() { return rootkitKext; }

		mach_port_t getKernelTaskPort() { return tfp0; }

		IOReturn createUserClient(task_t task, void *securityID, UInt32 type, IOKernelRootKitUserClient **client);
		IOReturn createUserClient(task_t task, void *securityID, UInt32 type, OSDictionary *properties, IOKernelRootKitUserClient **client);

		IOReturn newUserClient(task_t task, void *securityID, UInt32 type, OSDictionary *properties, IOUserClient **client);
		IOReturn newUserClient(task_t task, void *securityID, UInt32 type, IOUserClient **client);

		IOReturn addUserClient(IOKernelRootKitUserClient *client);

		IOReturn removeUserClient(IOKernelRootKitUserClient *client);

		IOReturn detachUserClients();

	private:
		mrk::MacRootKit *rootkit;

		xnu::Kernel *kernel;

		xnu::Kext *rootkitKext;

		mach_port_t tfp0;

		OSSet *userClients;
};

#endif