#include "PAC.hpp"

#ifdef defined(__arm64__) || defined(__arm64e__)

uint64_t Arch::arm64::PAC::signPointerWithAKey(uint64_t pointer)
{
	__asm__ volatile("PACIZA %[pac]" : [pac] "+rm" (pointer));

	return pointer;
}

uint64_t Arch::arm64::PAC::signPointerWithBKey(uint64_t pointer)
{
	__asm__ volatile("PACIZB %[pac]" : [pac] "+rm" (pointer));

	return pointer;
}

void Arch::arm64::PAC::stripPointerAuthenticationCode(uint64_t pointer)
{

}


#endif