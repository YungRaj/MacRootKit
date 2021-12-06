#include <stdint.h>
#include <string.h>

#include <kern/host.h>
#include <kern/task.h>

#include <mach/mach.h>
#include <mach/mach_types.h>
#include <mach/port.h>
#include <mach/kmod.h>
#include <mach/vm_types.h>

#include "IOKernelRootKitService.hpp"
#include "MacRootKit.hpp"

MacRootKit *rootkit = NULL;

MacRootKit* mac_rootkit_get_rootkit()
{
	if(rootkit)
		return rootkit;

	return NULL;
}

kern_return_t mac_rootkit_start(IOKernelRootKitService *service, Kernel *kernel, Kext **kext)
{
	kern_return_t ret = kIOReturnSuccess;

	rootkit = new MacRootKit(kernel);

	if(!rootkit)
		ret = kIOReturnUnsupported;

	return ret;
}

kern_return_t mac_rootkit_stop(IOKernelRootKitService * service, Kernel *kernel, Kext **kext)
{
	kern_return_t ret = kIOReturnSuccess;

	if(rootkit)
	{
		delete rootkit;

		rootkit = NULL
	}

	return ret;
}

extern "C"
{

	static kern_return_t mac_rootkit_kmod_start(struct kmod_info *ki, void *data)
	{
		MAC_RK_LOG("MacRK::kmod_start()!\n");

		return KERN_SUCCESS;
	}

	static kern_return_t mac_rootkit_kmod_stop(struct kmod_info *ki, void *data)
	{
		MAC_RK_LOG("MacRK::kmod_stop()!\n");

		return KERN_SUCCESS;
	}

	__private_extern__ kmod_start_func_t *_realmain = &mac_rootkit_kmod_start;
	__private_extern__ kmod_stop_func_t *_antimain = &mac_rootkit_kmod_stop

	extern kern_return_t _start(kmod_info_t *ki, void *data);
	extern kern_return_t _stop(kmod_info_t *ki, void *data);

	__attribute__((visibility("default"))) KMOD_EXPLICIT_DECL(net.siguza.hsp4, "1.0.0", _start, _stop);
}