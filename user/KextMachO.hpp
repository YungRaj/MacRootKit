#ifndef __KEXT_MACHO_HPP_
#define __KEXT_MACHO_HPP_

#include "MachO.hpp"

class MachO;

namespace xnu
{
	class Kernel;

	class KextMachO : public MachO 
	{
		public:
			KextMachO(uintptr_t base);
			KextMachO(uintptr_t base, off_t slide);
			
			KextMachO(const char *path, off_t slide);
			KextMachO(const char *path);

			~KextMachO();

			virtual void parseLinkedit();

			virtual bool parseLoadCommands();

			virtual void parseMachO();

		private:
			xnu::Kernel *kernel;
	};

};

#endif