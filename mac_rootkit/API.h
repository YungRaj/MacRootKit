#ifndef __API_H_
#define __API_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <assert.h>

enum kIOKernelRootKitOperation
{
	kIOKernelRootKitHookKernelFunction,
	kIOKernelRootKitAddBreakpoint,
	kIOKernelRootKitKernelCall,
	kIOKernelRootKitGetKaslrSlide,
	kIOKernelRootKitGetKernelBase,
	kIOKernelRootKitGetKernelSymbol,
	kIOKernelRootKitKernelRead,
	kIOKernelRootKitKernelReadUnsafe,
	kIOKernelRootKitKernelWrite,
	kIOKernelRootKitKernelWriteUnsafe,
	kIOKernelRootKitKernelVmAllocate,
	kIOKernelRootKitKernelVmDeallocate,
	kIOKernelRootKitKernelVmProtect,
	kIOKernelRootKitKernelVmRemap,
	kIOKernelRootKitKalloc,
	kIOKernelRootKitPhysicalRead,
	kIOKernelRootKitPhysicalWrite,
	kIOKernelRootKitKernelVirtualToPhysical,
	kIOKernelRootKitTaskForPid,
	kIOKernelRootKitMachVmRead,
	kIOKernelRootKitMachVmWrite,
	kIOKernelRootKitMachVmAllocate,
	kIOKernelRootKitMachVmProtect,
	kIOKernelRootKitMachVmCall,
	kIOKernelRootKitVirtualToPhysical,
	kIOKernelRootKitMmap,
	kIOKernelRootKitMachMsgSend,
	kIOKernelRootKitCopyIn,
	kIOKernelRootKitCopyOut,
	kIOKernelRootKitCreateSharedMemory,
	kIOKernelRootKitMapSharedMemory,
};

extern void* kern_os_malloc(size_t size);
extern void* kern_os_calloc(size_t num, size_t size);
extern void* kern_os_realloc(void *addr, size_t nsize);
extern void kern_os_free(void *addr);

extern void* memmem(const void *h0, size_t k, const void *n0, size_t l);
extern void* memchr(const void *src, int c, size_t n);
extern void  qsort(void *a, size_t n, size_t es, int (*cmp)(const void*, const void*));

#endif