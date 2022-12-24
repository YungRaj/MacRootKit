#include "Dwarf.hpp"




uint64_t Dwarf::ReadUleb128(uint8_t *p, uint8_t *end, uint32_t *idx)
{
	uint64_t result = 0;

	int bit = 0;

	do
	{
		if(p == end)
		{
			fprintf(stderr, "malformed uleb128\n");

			break;
		}

		uint64_t slice = *p & 0x7F;

		if(bit > 63)
		{
			fprintf(stderr, "uleb128 too big for uint64\n");

			break;
		} else 
		{
			result |= (slice << bit);

			bit += 7;
		}

		*idx = *idx + 1;

	} while(*p++ & 0x80);

	*idx = *idx - 1;

	return result;
}

int64_t Dwarf::ReadSleb128(uint8_t *p, uint8_t *end, uint32_t *idx)
{
	int64_t result = 0;

	int bit = 0;

	uint8_t byte = 0;

	do
	{
		if(p == end)
		{
			fprintf(stderr, "malformed sleb128\n");

			break;
		}

		byte = *p++;

		*idx = *idx + 1;

		result |= (((int64_t) (byte & 0x7f)) << bit);

		bit += 7;
	} while (byte & 0x80);
	// sign extend negative numbers

	*idx = *idx - 1;

	if(((byte & 0x40) != 0) && (bit < 64))
		result |= (~0ULL) << bit;

	return result;
}