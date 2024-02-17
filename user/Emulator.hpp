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

#include <Types.h>

#include <unicorn/unicorn.h>

namespace Emulation
{
	class Unicorn
	{
		public:
			explicit Unicorn(char *code, Size code_size, UInt64 address);

		private:
			uc_engine *uc;
	};


	class Panda
	{
		public:

		private:
	};

	template<typename Emu>
	class Emulator
	{
		public:
			explicit Emulator();

		private:
			Emu *emu;
	};
};
