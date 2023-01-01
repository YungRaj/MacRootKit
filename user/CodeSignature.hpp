#ifndef __CODE_SIGNATURE_HPP_
#define __CODE_SIGNATURE_HPP_

#include <mach/mach.h>
#include <mach/mach_types.h>

#include <sys/types.h>

#include <mach-o.h>

#include "MachO.hpp"

class MachO;

namespace mrk
{
	class CodeSignature
	{
		public:
			CodeSignature();

			static CodeSignature* codeSignatureWithLinkedit(MachO *macho, struct linkedit_data_command *cmd);

			bool verifyCodeSlot(uint8_t *blob, size_t size, bool sha256, char *signature, size_t sigsize);
			
			bool compareHash(uint8_t *hash1, uint8_t hash2, size_t hashSize);

			uint8_t* computeHash(bool sha256, uint8_t *blob, size_t size);

			bool parseCodeSignature();

		private:
			MachO *macho;

			SuperBlob *superBlob;

			code_directory_t code_directory;

			char *entitlements;

	};
};

#endif