#include "PAC.hpp"

#ifdef defined(__arm64__) || defined(__arm64e__)

UInt64 Arch::arm64::PAC::signPointerWithAKey(UInt64 pointer)
{
	__asm__ volatile("PACIZA %[pac]" : [pac] "+rm" (pointer));

	return pointer;
}

UInt64 Arch::arm64::PAC::signPointerWithBKey(UInt64 pointer)
{
	__asm__ volatile("PACIZB %[pac]" : [pac] "+rm" (pointer));

	return pointer;
}

void Arch::arm64::PAC::stripPointerAuthenticationCode(UInt64 pointer)
{

}


#endif