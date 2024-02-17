/*
 * Copyright (c) YungRaj
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

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