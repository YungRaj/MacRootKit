#ifndef __LOG_HPP_
#define __LOG_HPP_

#include "APIUtil.hpp"

extern "C"
{
	#include "API.h"
}

#ifdef __KERNEL_

#define MAC_PE_LOG IOLog

#endif

#ifdef __USER__

#define MAC_PE_LOG printf

#endif

#endif