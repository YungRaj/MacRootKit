#include "PAC.hpp"

#ifdef __arm64__

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