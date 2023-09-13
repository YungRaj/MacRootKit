#include "Fuzzer.hpp"
#include "Kernel.hpp"
#include "Log.hpp"

extern "C"
{
	#include "mach-o.h"
};

using namespace Fuzzer;

Harness::Harness(xnu::Kernel *kernel)
    : fuzzBinary(new FuzzBinary),
      kdkInfo(KDK::KDKInfoFromBuildInfo(kernel,
      									xnu::getOSBuildVersion(),
      									xnu::getKernelVersion()))
{
    loadKernel(kdkInfo->kernelPath, 0);
    addDebugSymbolsFromKernel(this->getBinary<KernelMachO*>(), kdkInfo->kernelDebugSymbolsPath);

    loader = new Loader(this->fuzzBinary);
}

Harness::Harness(const char *binary)
 : fuzzBinary(new FuzzBinary)
 {

 }

 Harness::Harness(const char *binary, const char *mapFile)
 : fuzzBinary(new FuzzBinary)
 {

 }

template <typename CpuType>
char* Harness::getMachOFromFatHeader(char *file_data)
{
    struct fat_header *header = reinterpret_cast<struct fat_header*>(file_data);

    swap_fat_header(header, NXHostByteOrder());

    struct fat_arch *arch = reinterpret_cast<struct fat_arch*>(file_data + sizeof(struct fat_header));

    swap_fat_arch(arch, header->nfat_arch, NXHostByteOrder());

    for(int i = 0; i < header->nfat_arch; i++)
    {
        uint32_t cputype;
        uint32_t cpusubtype;

        uint32_t offset;

        cputype = arch->cputype;
        cpusubtype = arch->cpusubtype;

        offset = arch->offset;

	#ifdef __arm64__

		static_assert(CpuType == CPU_TYPE_ARM64);

	#elif __x86_64__

		static_assert(CpuType == CPU_TYPE_X86_64);

	#endif

		if constexpr (std::is_same_v<CpuType, CPU_TYPE_ARM64>)
		{
			if (cputype == CPU_TYPE_ARM64)
			{
				return file_data + offset;
			}
		}

        if constexpr (std::is_same_v<CpuType, CPU_TYPE_X86_64>)
        {
            if (cputype == CPU_TYPE_X86_64)
            {
                return file_data + offset;
            }
        }

        arch++;
    }

    return NULL;
}

void Harness::addDebugSymbolsFromKernel(const char *debugSymbols)
{
	KernelMachO *macho = this->getBinary<KernelMachO*>();

	uintptr_t loadAddress = reinterpret_cast<uintptr_t>(this->fuzzBinary->base);
	uintptr_t oldLoadAddress = reinterpret_cast<uintptr_t>(this->fuzzBinary->originalBase);

	char *file_data;

    int fd = open(kernelPath, O_RDONLY);

    if(fd == -1)
    {
        MAC_RK_LOG("Error opening kernel Mach-O %s", kernelPath);

        return 0;
    }

    off_t file_size = lseek(fd, 0, SEEK_END);
    
    lseek(fd, 0, SEEK_SET);

    file_data = (char*) malloc(file_size);

    ssize_t bytes_read;

    bytes_read = read(fd, file_data, file_size);

    if(bytes_read != file_size)
    {
        MAC_RK_LOG("Read file failed!\n");

        close(fd);

        *outfile = NULL;

        return 0;
    }

    size_t total_size = 0;

    struct mach_header_64 *header = reinterpret_cast<struct mach_header_64*>(file_data);

    uint8_t *q = reinterpret_cast<uint8_t*>(file_data + sizeof(struct mach_header_64));

    for(int i = 0; i < header->ncmds; i++)
    {
        struct load_command *load_cmd = reinterpret_cast<struct load_command*>(q);

        uint32_t cmdtype = load_cmd->cmd;
        uint32_t cmdsize = load_cmd->cmdsize;

        if(cmdtype == LC_SYMTAB)
        {
            struct symtab_command *symtab_command = reinterpret_cast<struct symtab_command*>(load_cmd);

            struct nlist_64 *symtab = reinterpret_cast<struct nlist_64*>(file_data + symtab_command->symoff);
            uint32_t nsyms = symtab_command->nsyms;

            char *strtab = reinterpret_cast<char*>(file_data + symtab_command->stroff);
            uint32_t strsize = symtab_command->strsize;

            if(nsyms > 0)
            {
            	SymbolTable *symbolTable = macho->getSymbolTable();

				for(int i = 0; i < nsyms; i++)
				{
					Symbol *symbol;

					struct nlist_64 *nl = &symtab[i];

					char *name;

					mach_vm_address_t address;

					name = &strtab[nl->n_strx];

					address = (nl->n_value - oldLoadAddress) + loadAddress;

					nl->n_value = address;

				 	symbol = new Symbol(this, nl->n_type & N_TYPE, name, address, this->addressToOffset(address), this->segmentForAddress(address), this->sectionForAddress(address));

				 	symbolTable->replaceSymbol(symbol);
				}
            }
        }

        q += cmdsize;
    }

    free(file_data);
}

template<typename Binary>
void Harness::getMappingInfo(char *file_data, size_t *size, uintptr_t *load_addr)
{
    if constexpr (std::is_same_v<Binary, MachO>)
    {
        struct mach_header_64 *header = reinterpret_cast<struct mach_header_64*>(file_data);

        uint8_t *q = reinterpret_cast<uint8_t*>(file_data + sizeof(struct mach_header_64));

        for(int i = 0; i < header->ncmds; i++)
        {
            struct load_command *load_cmd = reinterpret_cast<struct load_command*>(q);

            uint32_t cmdtype = load_cmd->cmd;
            uint32_t cmdsize = load_cmd->cmdsize;

            if(cmdtype == LC_SEGMENT_64)
            {
                struct segment_command_64 *segment_command = reinterpret_cast<struct segment_command_64*>(load_cmd);

                mach_vm_address_t vmaddr = segment_command->vmaddr;

                if(vmaddr < *load_addr && strcmp(segment_command->segname, "__TEXT") == 0)
                    *load_addr = vmaddr;

                *size += segment_command->vmsize;
            }

            q += cmdsize;
        }
    }

    if constexpr (std::is_same_v<Binary, RawBinary>)
    {

    }    
}

void Harness::updateSymbolTableForMappedMachO(char *file_data, uintptr_t newLoadAddress, uintptr_t oldLoadAddress)
{
    struct mach_header_64 *header = reinterpret_cast<struct mach_header_64*>(file_data);

    uint8_t *q = reinterpret_cast<uint8_t*>(file_data + sizeof(struct mach_header_64));

    for(int i = 0; i < header->ncmds; i++)
    {
        struct load_command *load_cmd = reinterpret_cast<struct load_command*>(q);

        uint32_t cmdtype = load_cmd->cmd;
        uint32_t cmdsize = load_cmd->cmdsize;

        if(cmdtype == LC_SYMTAB)
        {
            struct symtab_command *symtab_command = reinterpret_cast<struct symtab_command*>(load_cmd);

            struct nlist_64 *symtab = reinterpret_cast<struct nlist_64*>(file_data + symtab_command->symoff);
            uint32_t nsyms = symtab_command->nsyms;

            char *strtab = reinterpret_cast<char*>(file_data + symtab_command->stroff);
            uint32_t strsize = symtab_command->strsize;

            if(nsyms > 0)
            {
                for(int i = 0; i < nsyms; i++)
                {
                    struct nlist_64 *nl = &symtab[i];

                    char *name;

                    mach_vm_address_t address;

                    name = &strtab[nl->n_strx];

                    address = (nl->n_value - oldLoadAddress) + newLoadAddress;

                    printf("Symbol %s = 0x%llx\n", name, address);

                    nl->n_value = address;
                }
            }
        }

        q += cmdsize;
    };
}

void Harness::updateSegmentLoadCommandsForNewLoadAddress(char *file_data, uintptr_t newLoadAddress, uintptr_t oldLoadAddress)
{
    struct mach_header_64 *header = reinterpret_cast<struct mach_header_64*>(file_data);

    uint8_t *q = reinterpret_cast<uint8_t*>(file_data + sizeof(struct mach_header_64));

    for(int i = 0; i < header->ncmds; i++)
    {
        struct load_command *load_cmd = reinterpret_cast<struct load_command*>(q);

        uint32_t cmdtype = load_cmd->cmd;
        uint32_t cmdsize = load_cmd->cmdsize;

        if(cmdtype == LC_SEGMENT_64)
        {
            struct segment_command_64 *segment_command = reinterpret_cast<struct segment_command_64*>(load_cmd);

            mach_vm_address_t vmaddr = segment_command->vmaddr;

            mach_vm_address_t segment_adjusted_address = segment_command->fileoff + newLoadAddress;

            segment_command->vmaddr = segment_adjusted_address;

            printf("LC_SEGMENT_64 at 0x%llx - %s 0x%08llx to 0x%08llx \n", segment_command->fileoff,
                                          segment_command->segname,
                                          segment_command->vmaddr,
                                          segment_command->vmaddr + segment_command->vmsize);

            uint8_t *sects  = q + sizeof(struct segment_command_64);

            for(int j = 0; j < segment_command->nsects; j++)
            {
                struct section_64 *section = reinterpret_cast<struct section_64*>(sects);

                mach_vm_address_t sect_addr = section->addr;

                mach_vm_address_t sect_adjusted_address = (sect_addr - vmaddr) + segment_command->fileoff + newLoadAddress;

                section->addr = sect_adjusted_address;
                section->offset = (sect_addr - vmaddr) + segment_command->fileoff;

                printf("\tSection %d: 0x%08llx to 0x%08llx - %s\n", j,
                                                section->addr,
                                                section->addr + section->size,
                                                section->sectname);


                memcpy((void*) sect_adjusted_address, file_data + section->offset, section->size);

                sects += sizeof(struct section_64);
            };
        }

        q += cmdsize;
    }
}

template<typename Binary>
bool Harness::mapSegments(char *file_data, char *mapFile)
{
    if constexpr (std::is_same_v<Binary, MachO>)
    {
        struct mach_header_64 *header = reinterpret_cast<struct mach_header_64*>(file_data);

        uint8_t *q = reinterpret_cast<uint8_t*>(file_data + sizeof(struct mach_header_64));

        for(int i = 0; i < header->ncmds; i++)
        {
            struct load_command *load_cmd = reinterpret_cast<struct load_command*>(q);

            uint32_t cmdtype = load_cmd->cmd;
            uint32_t cmdsize = load_cmd->cmdsize;

            if(cmdtype == LC_SEGMENT_64)
            {
                struct segment_command_64 *segment_command = reinterpret_cast<struct segment_command_64*>(load_cmd);

                printf("LC_SEGMENT_64 at 0x%llx - %s 0x%08llx to 0x%08llx \n", segment_command->fileoff,
                                              segment_command->segname,
                                              segment_command->vmaddr,
                                              segment_command->vmaddr + segment_command->vmsize);

                if (mprotect((void*) segment_command->vmaddr, segment_command->vmsize, PROT_READ | PROT_WRITE) == -1)
                {
                    printf("mprotect(R/W) failed!\n");

                    return false;
                }

                memcpy((void*) segment_command->vmaddr, file_data + segment_command->fileoff, segment_command->filesize);

                uint8_t *sects  = q + sizeof(struct segment_command_64);

                for(int j = 0; j < segment_command->nsects; j++)
                {
                    struct section_64 *section = reinterpret_cast<struct section_64*>(sects);

                    mach_vm_address_t sect_addr = section->addr;

                    printf("\tSection %d: 0x%08llx to 0x%08llx - %s\n", j,
                                                    section->addr,
                                                    section->addr + section->size,
                                                    section->sectname);

                    memcpy((void*) section->addr, file_data + section->offset, section->size);

                    sects += sizeof(struct section_64);
                };

                if (mprotect((void*) segment_command->vmaddr, segment_command->vmsize, segment_command->maxprot) == -1)
                {
                    printf("mprotect(maxprot) failed!\n");

                    return false;
                }
            }

            q += cmdsize;

            return true;
    }
    }

    if constexpr (std::is_same_v<Binary, RawBinary>)
    {

    }

    return false;
}

template<typename Binary>
bool Harness::unmapSegments()
{

}

void Harness::loadKernelMachO(const char *kernelPath, uintptr_t *loadAddress, size_t *loadSize, uintptr_t *oldLoadAddress)
{
    bool success;

    char *file_data;

    int fd = open(kernelPath, O_RDONLY);

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
    #ifdef __arm64__
        file_data = this->getMachOFromFatHeader<CPU_TYPE_ARM64>(file_data);
    #elif __x86_64__
        file_data = this->getMachOFromFatHeader<CPU_TYPE_X86_64>(file_data);
    #endif
    }

    *oldLoadAddress = UINT64_MAX;

    getMappingInfoForMachO(file_data, loadSize, oldLoadAddress);

    if(*oldLoadAddress == UINT64_MAX)
    {
        printf("oldLoadAddress == UINT64_MAX");

        close(fd);

        *loadAddress = 0;
        *loadSize = 0;
        *oldLoadAddress = 0;

        return;
    }

    void* baseAddress = mmap(NULL, *loadSize, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    
    if(baseAddress == MAP_FAILED)
    {
        printf("mmap() failed!\n");

        close(fd);

        *loadAddress = 0;
        *loadSize = 0;
        *oldLoadAddress = 0;

        return;
    }

    this->updateSymbolTableForMappedMachO(file_data, (uintptr_t) baseAddress, *oldLoadAddress);

    this->updateSegmentLoadCommandsForNewLoadAddress(file_data, (uintptr_t) baseAddress, *oldLoadAddress);

    success = this->mapSegments<MachO>(file_data, NULL);

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

    printf("Load Kernel MachO failed!\n");
}

template<typename Binary>
void Harness::loadBinary(const char *path, const char *mapFile)
{

}

void Harness::loadKernel(const char *kernelPath, off_t slide)
{
    uintptr_t loadAddress = 0;
    uintptr_t oldLoadAddress = 0;

    size_t loadSize = 0;

    loadAddress = this->loadKernelMachO(kernelPath, &loadAddress, &loadSize, &oldLoadAddress);

    this->fuzzBinary->path = kernelPath;
    this->fuzzBinary->base = reinterpret_cast<void*>(loadAddress);
    this->fuzzBinary->originalBase = reinterpret_cast<void*>(oldLoadAddress);
    this->fuzzBinary->size = loadSize;
    this->fuzzBinary->binary = MakeBinary<KernelMachO*>(new KernelMachO(loadAddress));
}


void Harness::loadKernelExtension(const char *path)
{
	this->loader->loadModuleFromKext(path);
}

template<typename Binary>
void Harness::populateSymbolsFromMapFile(const char *mapFile)
{

}

template <typename T>
void Harness::mutate(T data) requires FuzzableType<T>
{
	if constexpr (ScalarType<T>)
	{
        
    } else {
        
    }
}

template<typename Func, typename... Args, typename Binary, typename Sym>
std::invoke_result_t<Func, Args...> Harness::execute(const char *name, Func func, Args... args)
{

}
