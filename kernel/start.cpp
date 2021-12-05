#include <mach/mach.h>
#include <mach/mach_types.h>
#include <mach/kmod.h>

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