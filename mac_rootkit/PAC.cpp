#include "PAC.hpp"

#ifdef __arm64__

uint64_t PAC::signPointerWithAKey(uint64_t pointer)
{
	__asm__ volatile("PACIZA %[pac]" : [pac] "+rm" (pointer));

	return pointer;
}

uint64_t PAC::signPointerWithBKey(uint64_t pointer)
{
	__asm__ volatile("PACIZB %[pac]" : [pac] "+rm" (pointer));

	return pointer;
}


#endif