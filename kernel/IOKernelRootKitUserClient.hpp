#ifndef __IOKERNELRootKitUSERCLIENT_HPP_
#define __IOKERNELRootKitUSERCLIENT_HPP_

#include <IOKit/IOLib.h>
#include <IOKit/IOUserClient.h>

#include <mach/mach_types.h>
#include <mach/vm_types.h>

#include "APIUtil.hpp"

#include "IOKernelRootKitService.hpp"

#include "Kernel.hpp"

namespace mrk
{
	class MacRootKit;
}

using namespace xnu;
using namespace mrk;

class IOKernelRootKitService;

class IOKernelRootKitUserClient : public IOUserClient
{
	OSDeclareDefaultStructors(IOKernelRootKitUserClient)

	public:
		static IOKernelRootKitUserClient* rootKitUserClientWithKernel(xnu::Kernel *kernel, task_t owningTask, void *securityToken, UInt32 type);

		static IOKernelRootKitUserClient* rootKitUserClientWithKernel(xnu::Kernel *kernel, task_t owningTask, void *securityToken, UInt32 type, OSDictionary *properties);

		virtual bool initRootKitUserClientWithKernel(xnu::Kernel *kernel, task_t owningTask, void *securityToken, UInt32 type);

		virtual bool initRootKitUserClientWithKernel(xnu::Kernel *kernel, task_t owningTask, void *securityToken, UInt32 type, OSDictionary *properties);

		virtual bool start(IOService *provider);
		virtual void stop(IOService *provider);

		virtual IOReturn clientClose();
		virtual IOReturn clientDied();

		virtual void free();

		virtual IOExternalMethod* getExternalMethodForIndex(UInt32 index);
		virtual IOExternalTrap*   getExternalTrapForIndex(UInt32 index);

		virtual IOReturn externalMethod(UInt32 selector, IOExternalMethodArguments *arguments, IOExternalMethodDispatch *dispatch, OSObject *target, void *reference);

		IOKernelRootKitService* getRootKitService() { return rootkitService; }

		task_t getClientTask() { return clientTask; }
		task_t getKernelTask() { return kernelTask; }

	private:
		IOKernelRootKitService *rootkitService;

		task_t clientTask;

		task_t kernelTask;

		xnu::Kernel *kernel;

		void initRootKit();

		uint8_t* mapBufferFromClientTask(mach_vm_address_t uaddr, size_t size, IOOptionBits options, IOMemoryDescriptor **desc, IOMemoryMap **mapping);

};

#endif