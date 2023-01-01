#include <stdlib.h>
#include <string.h>

#include <CommonCrypto/CommonDigest.h>

#include "CodeSignature.hpp"

using namespace mrk;

CodeSignature* CodeSignature::codeSignatureWithLinkedit(MachO *macho, struct linkedit_data_command *cmd)
{
	CodeSignature *signature = NULL;

	return signature;
}

bool CodeSignature::verifyCodeSlot(uint8_t *blob, size_t size, bool sha256, char *signature, size_t signature_size)
{
	bool verified = false;

	if(sha256)
	{
		unsigned char result[CC_SHA256_DIGEST_LENGTH];

		CC_SHA256(blob, static_cast<uint32_t> (size), result);

		verified = (memcmp(result, signature, min(CC_SHA256_DIGEST_LENGTH, signature_size)));
	} else
	{
		unsigned char result[CC_SHA1_DIGEST_LENGTH];

		CC_SHA1(blob, static_cast<uint32_t> (size), result);

		verified = (memcmp(result, signature, min(CC_SHA1_DIGEST_LENGTH, signature_size)));
	}

	return verified;
}
	
bool CodeSignature::compareHash(uint8_t *hash1, uint8_t hash2, size_t hashSize)
{
	const char *h1 = reinterpret_cast<const char*>(hash1);
	const char *h2 = reinterpret_cast<const char*>(hash2);

	return (memcmp(h1, h2, hashSize) == 0);
}

uint8_t* CodeSignature::computeHash(bool sha256, uint8_t *blob, size_t size)
{
	uint8_t *result;

	if(sha256)
	{
		result = static_cast<uint8_t*>(malloc(CC_SHA256_DIGEST_LENGTH));

		CC_SHA256(blob, static_cast<uint32_t> (size), result);
	} else
	{
		result =  static_cast<uint8_t*>(malloc(CC_SHA1_DIGEST_LENGTH));

		CC_SHA1(blob, static_cast<uint32_t> (size), result);
	}

	return result;
}


bool CodeSignature::parseCodeSignature()
{
	SuperBlob *superblob;

	return true;
}
