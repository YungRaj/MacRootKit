#include "Loader.hpp"
#include "Architecture.hpp"
#include "Log.hpp"

#include <sys/mman.h>

using namespace Fuzzer;

void Module::load()
{
	
}

template<typename Sym>
Sym Module::getSymbol(const char *symname) requires requires(Sym sym) {
    { sym.getName() };
    { sym.getAddress() };
}
{

}

template<typename Seg>
void Module::mapSegment(Seg segment) requires requires(Seg seg) {
    { seg.getAddress() };
}
{

}

template<typename Sect>
void Module::mapSection(Sect section) requires requires(Sect sect) {
    { sect.getAddress() };
    { sect.getSize() };
}
{

}

Loader::Loader(struct FuzzBinary *binary)
{

}

void* Loader::allocateSegmentMemory(uintptr_t addr, size_t sz, int prot)
{
    size_t size = sz;

    void* segmentAddress = reinterpret_cast<void*>(addr); // Specify the desired address

    void* segmentMappedMemory = mmap(segmentAddress, size, prot, MAP_PRIVATE | MAP_ANON | MAP_FIXED, -1, 0);

    if (segmentMappedMemory == MAP_FAILED)
    {
        MAC_RK_LOG("Fuzzer::Loader::allocateSegmentMemory() mmap(0x%llx, 0x%x, %d) failed!", address, sz, prot);

        return false;
    }
}

void Loader::linkSymbols(Module *module)
{

}

void Loader::linkSymbol(Module *module, Symbol *symbol)
{

}

void Loader::stubFunction(Module *module, Symbol *symbol, uintptr_t stub)
{

}