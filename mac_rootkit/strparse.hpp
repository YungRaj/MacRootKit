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

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#ifndef __KERNEL__

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#endif

int hex_digit(char c);

enum strparse_result {
	STRPARSE_OK,
	STRPARSE_BADDIGIT,
	STRPARSE_NODIGITS,
	STRPARSE_OVERFLOW,
};

char* strnchar(char *str, uint32_t len, char ch);

enum strtoint_result {
	STRTOINT_OK,
	STRTOINT_BADDIGIT,
	STRTOINT_NODIGITS,
	STRTOINT_OVERFLOW,
};

enum strtoint_result strtoint(char *str,
							  uint32_t len,
							  bool sign,
							  bool is_signed,
							  uint32_t base,
							  uint64_t *value,
							  char **end);

enum strtodata_result {
	STRTODATA_OK,
	STRTODATA_BADBASE,
	STRTODATA_BADDIGIT,
	STRTODATA_NEEDDIGIT,
	STRTODATA_NODIGITS,
};

enum strtodata_result strtodata(char *str,
								uint32_t base,
								void *data,
								uint32_t *size,
								char **end);

enum strparse_result strreplace(char *str, char find, char replace);

#ifdef __KERNEL__

extern "C"
{
	char* strdup(char *s);

	char* strstr(char *string, char *substring);
}

#endif

char* strtokmul(char *input, char *delimiter);

char *trim(char *s);

char *deblank(char *input);

