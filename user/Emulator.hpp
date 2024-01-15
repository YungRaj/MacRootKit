#ifndef __EMULATOR_HPP_
#define __EMULATOR_HPP_

#include <unicorn/unicorn.h>

namespace Emulation
{
	class Unicorn
	{
		public:
			explicit Unicorn(char *code, size_t code_size, uint64_t address);

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

#endif