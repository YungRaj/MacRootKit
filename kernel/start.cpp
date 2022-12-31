#include <stdint.h>
#include <string.h>

#include <kern/host.h>
#include <kern/task.h>

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
	{
		ret = kIOReturnUnsupported;
	}

	// *kext = rootkit->getKextByIdentifier("com.YungRaj.MacRootKit");

	if(!kext)
	{
		MAC_RK_LOG("MacRK::mac_rootkit_start() cannot find com.YungRaj.MacRootKit kext!\n");
	} else
	{
		MAC_RK_LOG("MacRK::mac_rootkit_start() found com.YungRaj.MacRootKit kext!\n");
	}

	return ret;
}

kern_return_t mac_rootkit_stop(IOKernelRootKitService * service, Kernel *kernel, Kext **kext)
{
	kern_return_t ret = kIOReturnSuccess;

	if(rootkit)
	{
		delete rootkit;

		rootkit = NULL;
	}

	return ret;
}

extern "C"
{
	kern_return_t kern_start(kmod_info_t *ki, void *data)
	{	
		MAC_RK_LOG("MacRK::kmod_start()!\n");

		return KERN_SUCCESS;
	}

	kern_return_t kern_stop(kmod_info_t *ki, void *data)
	{
		MAC_RK_LOG("MacRK::kmod_stop()!\n");

		return KERN_SUCCESS;
	}

	extern kern_return_t _start(kmod_info_t *, void*);
	extern kern_return_t _stop(kmod_info_t *, void*);

	__private_extern__ kmod_start_func_t *_realmain = kern_start;
	__private_extern__ kmod_stop_func_t *_antimain = kern_stop;

	__attribute__((visibility("default"))) KMOD_EXPLICIT_DECL(com.YungRaj.MacRootKit, "1.0.1", _start, _stop);

	__private_extern__ int _kext_apple_;
}