#ifndef __API_UTIL_HPP_
#define __API_UTIL_HPP_

#ifdef __USER__

#include <IOKit/IOKitLib.h>

#endif

#ifdef __KERNEL__

#include <IOKit/IOLib.h>

#endif

#ifdef __cplusplus

	extern "C"
	{
#endif
		#include "API.h"
#ifdef __cplusplus
	}
#endif

class Kernel;

/**
 *  Export function or symbol for linking
 */
#define EXPORT __attribute__((visibility("default")))

/**
 *  Ensure the symbol is not exported
 */
#define PRIVATE __attribute__((visibility("hidden")))

/**
 *  For private fallback symbol definition
 */
#define WEAKFUNC __attribute__((weak))

/**
 *  Remove padding between fields
 */
#define PACKED __attribute__((packed))

/**
 *  Deprecate the interface
 */
#define DEPRECATE(x) __attribute__((deprecated(x)))

/**
 *  Non-null argument
 */
#define NONNULL __attribute__((nonnull))

namespace API
{
	void dump(Kernel *kernel, mach_vm_address_t address);

	void hexdump(unsigned char *data, size_t size, bool rev);
};

#endif