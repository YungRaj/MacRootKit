#include "IOKernelRootKitService.hpp"
#include "MacRootKit.hpp"

kern_return_t mac_rootkit_start(IOKernelRootKitService *service, Kernel *kernel, Kext *kext)
{
	return KERN_SUCCESS;
}

kern_return_t mac_rootkit_stop(IOKernelRootKitService * service, Kernel *kernel, Kext *kext)
{
	return KERN_SUCCESS;
}

