#ifndef __IOKERNELRootKitUSERCLIENT_HPP_
#define __IOKERNELRootKitUSERCLIENT_HPP_

#include <IOKit/IOLib.h>

#include <mach/mach.h>
#include <mach/mach_types.h>
#include <mach/vm_types.h>

#include "APIUtil.hpp"

#include "IOKernelRootKitService.hpp"

#include "Kernel.hpp"

class IOKernelRootKitService;
class Kernel;

class IOKernelRootKitUserClient : public IOUserClient
{
	OSDeclareDefaultStructors(IOKernelRootKitUserClient)

	public:
		static IOKernelRootKitUserClient* rootKitUserClientWithKernel(Kernel *kernel, task_t owningTask, void *securityToken, UInt32 type);

		static IOKernelRootKitUserClient* rootKitUserClientWithKernel(Kernel *kernel, task_t owningTask, void *securityToken, UInt32 type, OSDictionary *properties);

		virtual bool initRootKitUserClientWithKernel(Kernel *kernel, task_t owningTask, void *securityToken, UInt32 type);

		virtual bool initRootKitUserClientWithKernel(Kernel *kernel, task_t owningTask, void *securityToken, UInt32 type, OSDictionary *properties);

		virtual bool start(IOService *provider);
		virtual bool stop(IOService *provider);

		virtual IOReturn clientClose();
		virtual IOReturn clientDied();

		virtual void free();

		virtual IOExternalMethod* getExternalMethodForIndex(UInt32 index);
		virtual IOExternalTrap*   getExternalTrapForIndex(UInt32 index);

		virtual IOReturn externalMethod(UInt32 selector, IOExternalMethodArguments *arguments, IOExternalMethodDispatch *dispatch, OSObject *target, void *reference);

		IOKernelRootKitService* getRootKitService() { return RootKitService; }

		task_t getClientTask() { return clientTask; }
		task_t getKernelTask() { return kernelTask; }

	private:
		IOKernelRootKitService *RootKitService;

		task_t clientTask;

		task_t kernelTask;

		Kernel *kernel;

		void initRootKit();

		uint8_t* mapBufferFromUserspace(mach_vm_address_t uaddr, size_t size, IOOptionBits options);

};

#endif