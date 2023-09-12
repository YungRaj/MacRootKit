#include "Loader.hpp"
#include "KernelMachO.hpp"
#include "KextMachO.hpp"
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

Loader::Loader(Fuzzer::Harness *harness, struct FuzzBinary *binary)
    : architecture(Architecture::getArchitecture()),
      harness(harness),
      binary(binary)
{
    
}

void Loader::loadModuleFromKext(const char *kextPath)
{
    Fuzzer::Module *module;

    uintptr_t loadAddress = 0;
    uintptr_t oldLoadAddress = 0;

    char *file_data = NULL;

    size_t file_size = 0;

    loadAddress = loadKextMacho(kextPath, &file_data, &file_size, &oldLoadAddress);

    fuzzBinary->path = kextPath;
    fuzzBinary->base = reinterpret_cast<void*>(loadAddress);
    fuzzBinary->originalBase = reinterpret_cast<void*>(oldLoadAddress);
    fuzzBinary->size = size;
    fuzzBinary->binary = MakeBinary<KextMachO*>(new KextMachO(loadAddress));

    module = new Module(kextPath, fuzzBinary);
}

void Loader::loadKextMachO(const char *kextPath, uintptr_t *loadAddress, size_t *loadSize, uintptr_t *oldLoadAddress);
{
    bool success;

    char *file_data;

    int fd = open(kextPath, O_RDONLY);

    if(fd == -1)
    {
        printf("Error opening kernel Mach-O %s", kernelPath);

        *loadAddress = 0;
        *loadSize = 0;
        *oldLoadAddress = 0;

        return;
    }

    off_t file_size = lseek(fd, 0, SEEK_END);
    
    lseek(fd, 0, SEEK_SET);

    file_data = (char*) malloc(file_size);

    ssize_t bytes_read;

    bytes_read = read(fd, file_data, file_size);

    if(bytes_read != file_size)
    {
        printf("Read file failed!\n");

        close(fd);

        *loadAddress = 0;
        *loadSize = 0;
        *oldLoadAddress = 0;

        return;
    }

    if(reinterpret_cast<struct mach_header_64*>(file_data)->magic == FAT_CIGAM)
    {
        file_data = this->harness->getMachOFromFatHeader(file_data);
    }

    *oldLoadAddress = UINT64_MAX;

    this->harness->getMappingInfoForMachO(file_data, loadSize, oldLoadAddress);

    if(*oldLoadAddress == UINT64_MAX)
    {
        printf("oldLoadAddress == UINT64_MAX");

        close(fd);

        *loadAddress = 0;
        *loadSize = 0;
        *oldLoadAddress = 0;

        return;
    }

    void *baseAddress = this->allocateModuleMemory(*loadSize, PROT_READ | PROT_WRITE);
    
    if(baseAddress == MAP_FAILED)
    {
        printf("mmap() failed!\n");

        close(fd);

        *loadAddress = 0;
        *loadSize = 0;
        *oldLoadAddress = 0;

        return;
    }

    this->harness->updateSymbolTableForMappedMachO(file_data, (uintptr_t) baseAddress, *oldLoadAddress);

    this->harness->updateSegmentLoadCommandsForNewLoadAddress(file_data, (uintptr_t) baseAddress, *oldLoadAddress);

    success = this->harness->mapSegmentsFromMachO(file_data);

    if(!success)
    {
        printf("Map Segments failed!\n");

        goto fail;
    }

    *loadAddress = (uintptr_t) baseAddress;

    // writeToFile((char*) baseAddress, *loadSize);

    free(file_data);

    close(fd);

    return;

fail:
    close(fd);

    *loadAddress = 0;
    *loadSize = 0;
    *oldLoadAddress = 0;

    printf("Load Kext MachO failed!\n");
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

void* Loader::allocateModuleMemory(size_t sz, int prot)
{
    void* baseAddress = mmap(NULL, size, prot, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    return baseAddress;
}