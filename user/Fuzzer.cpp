#include "Fuzzer.hpp"
#include "Log.hpp"

using namespace Fuzzer;

Harness::Harness()
{
	this->fuzzBinary = new FuzzBinary;
	this->loader = new Loader(this->fuzzBinary);

	this->loadKernel("/Library/Developer/KDKs/KDK_13.3.1_22E261.kdk/System/Library/Kernels/kernel.release.t6000", 0);
	this->addDebugSymbolsFromKernel(this->getBinary<KernelMachO*>(), "/Library/Developer/KDKs/KDK_13.3.1_22E261.kdk/System/Library/Kernels/kernel.release.t6000.dSYM/Contents/Resources/DWARF/kernel.release.t6000");
}

void Harness::addDebugSymbolsFromKernel(KernelMachO *macho, const char *debugSymbols)
{
	char *file_data;

    int fd = open(kernelPath, O_RDONLY);

    if(fd == -1)
    {
        printf("Error opening kernel Mach-O %s", kernelPath);

        return 0;
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

					address = nl->n_value;

				 	symbol = new Symbol(this, nl->n_type & N_TYPE, name, address, this->addressToOffset(address), this->segmentForAddress(address), this->sectionForAddress(address));

				 	if(!symbolTable->containsSymbolNamed(name))
						symbolTable->addSymbol(symbol);
				}
            }
        }

        q += cmdsize;
    }

    free(file_data);
}

uintptr_t Harness::mapSegmentsFromKernelMachO(const char *kernelPath, char **outfile)
{
    char *file_data;

    int fd = open(kernelPath, O_RDONLY);

    if(fd == -1)
    {
        printf("Error opening kernel Mach-O %s", kernelPath);

        return 0;
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

        *outfile = NULL;

        return 0;
    }

    uintptr_t loadAddress = UINT64_MAX;

    size_t total_size = 0;

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

            if(vmaddr < loadAddress && strcmp(segment_command->segname, "__TEXT") == 0)
                loadAddress = vmaddr;

            total_size += segment_command->vmsize;
        }

        q += cmdsize;
    }

    if(loadAddress == UINT64_MAX)
    {
        printf("loadAddress == UINT64_MAX");

        close(fd);

        return loadAddress;
    }

    void* baseAddress = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    
    if(baseAddress == MAP_FAILED)
    {
        printf("mmap() failed!\n");

        goto fail;
    }

    q = reinterpret_cast<uint8_t*>(file_data + sizeof(struct mach_header_64));

    for(int i = 0; i < header->ncmds; i++)
    {
        struct load_command *load_cmd = reinterpret_cast<struct load_command*>(q);

        uint32_t cmdtype = load_cmd->cmd;
        uint32_t cmdsize = load_cmd->cmdsize;

        if(cmdtype == LC_SEGMENT_64)
        {
            struct segment_command_64 *segment_command = reinterpret_cast<struct segment_command_64*>(load_cmd);

            mach_vm_address_t vmaddr = segment_command->vmaddr;

            mach_vm_address_t segment_adjusted_address = (vmaddr - loadAddress) + (mach_vm_address_t) baseAddress;

            segment_command->vmaddr = segment_adjusted_address;

            printf("LC_SEGMENT_64 at 0x%llx - %s 0x%08llx to 0x%08llx \n", segment_command->fileoff,
                                          segment_command->segname,
                                          segment_command->vmaddr,
                                          segment_command->vmaddr + segment_command->vmsize);

            if (mprotect((void*) segment_adjusted_address, segment_command->vmsize, PROT_READ | PROT_WRITE) == -1)
            {
                printf("mprotect() failed!\n");

                goto fail;
            }

            memcpy((void*) segment_adjusted_address, file_data + segment_command->fileoff, segment_command->filesize);

            uint8_t *sects  = q + sizeof(struct segment_command_64);

            for(int j = 0; j < segment_command->nsects; j++)
            {
                struct section_64 *section = reinterpret_cast<struct section_64*>(sects);

                mach_vm_address_t sect_addr = section->addr;

                mach_vm_address_t sect_adjusted_address = (sect_addr - loadAddress) + (mach_vm_address_t) baseAddress;

                section->addr = sect_adjusted_address;

                printf("\tSection %d: 0x%08llx to 0x%08llx - %s\n", j,
                                                section->addr,
                                                section->addr + section->size,
                                                section->sectname);


                memcpy((void*) sect_adjusted_address, file_data + section->offset, section->size);

                sects += sizeof(struct section_64);
            };

            if (mprotect((void*) segment_adjusted_address, segment_command->vmsize, segment_command->maxprot) == -1)
            {
                printf("mprotect() failed!\n");

                goto fail;
            }
        }

        q += cmdsize;
    }

    close(fd);

    *outfile = file_data;

    return (uintptr_t) baseAddress;

fail:
    close(fd);

    printf("Map segments Mach-O failed!\n");

    *outfile = NULL;

    return 0;
}

void Harness::loadBinary(const char *path, const char *symbolsFile)
{

}

void Harness::loadKernel(const char *kernelPath, off_t slide)
{
    uintptr_t loadAddress = 0;

    char *file_data = NULL;

    loadAddress = mapSegmentsFromKernelMachO(kernelPath, &file_data);

    this->fuzzBinary.binary.macho = new KernelMachO(loadAddress);
}


void Harness::loadKernelExtension(const char *path)
{

}

void Harness::populateSymbols(const char *symbolsFile)
{

}