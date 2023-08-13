#ifndef __ASSEMBLE_H_
#define __ASSEMBLE_H_

namespace Arch
{
	namespace arm64
	{
		namespace Assembler
		{
			uint32_t* assemble(char *ins, uint32_t *nins);
			uint32_t assembleInstruction(char *ins);
		};
	};
};

#endif