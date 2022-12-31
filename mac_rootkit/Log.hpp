#ifndef __LOG_HPP_
#define __LOG_HPP_

#include "APIUtil.hpp"

extern "C"
{
	#include "API.h"
}

#ifdef __KERNEL__

#include <os/log.h>

#define MAC_RK_LOG(...) os_log(OS_LOG_DEFAULT, __VA_ARGS__)

#endif

#ifdef __USER__

#define MAC_RK_LOG printf

#endif

#endif