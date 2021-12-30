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
	static kern_return_t mac_rootkit_kmod_start(struct kmod_info *ki, void *data)
	{
		mach_vm_address_t kernel_base = Kernel::findKernelBase();

		off_t kernel_slide = 0; //Kernel::findKernelSlide();

		char buffer[128];

		snprintf(buffer, 128, "0x%llx", kernel_base);

		MAC_RK_LOG("MacRK::IOKernelRootKitService::kernel_base = %s\n", buffer);

		snprintf(buffer, 128, "0x%llx", kernel_slide);

		MAC_RK_LOG("MacRK::IOKernelRootKitService::kernel_slide = %s\n", buffer);
		
		MAC_RK_LOG("MacRK::kmod_start()!\n");

		return KERN_SUCCESS;
	}

	static kern_return_t mac_rootkit_kmod_stop(struct kmod_info *ki, void *data)
	{
		MAC_RK_LOG("MacRK::kmod_stop()!\n");

		return KERN_SUCCESS;
	}

	__private_extern__ kmod_start_func_t *_realmain = &mac_rootkit_kmod_start;
	__private_extern__ kmod_stop_func_t *_antimain = &mac_rootkit_kmod_stop;

	// __attribute__((visibility("default"))) KMOD_EXPLICIT_DECL(com.YungRaj.MacRootKit, "1.0.1", mac_rootkit_kmod_start, mac_rootkit_kmod_stop);
}