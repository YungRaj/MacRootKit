#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h> 
#include <string.h> 
#include <dirent.h>

#include <assert.h>

#include <mach/mach_types.h>

#include <sys/sysctl.h>
#include <sys/utsname.h>

extern "C"
{
    #include "../mac_rootkit/mach-o.h"
}

#define swap32(x) OSSwapInt32(x)

enum KDKKernelType
{
    KdkKernelTypeNone = -1,
    KdkKernelTypeRelease = 0,
    KdkKernelTypeReleaseT6000,
    KdkKernelTypeReleaseT6020,
    KdkKernelTypeReleaseT8103,
    KdkKernelTypeReleaseT8112,
    KdkKernelTypeReleaseVmApple,

    KdkKernelTypeDevelopment = 0x10,
    KdkKernelTypeDevelopmentT6000,
    KdkKernelTypeDevelopmentT6020,
    KdkKernelTypeDevelopmentT8103,
    KdkKernelTypeDevelopmentT8112,
    KdkKernelTypeDevelopmentVmApple,

    KdkKernelTypeKasan = 0x20,
    KdkKernelTypeKasanT6000,
    KdkKernelTypeKasanT6020,
    KdkKernelTypeKasanT8103,
    KdkKernelTypeKasanT8112,
    KdkKernelTypeKasanVmApple,
};

#define KDK_PATH_SIZE 1024

struct KDKInfo
{
    KDKKernelType type;

    char *kernelName;

    char path[KDK_PATH_SIZE];
    char kernelPath[KDK_PATH_SIZE];
    char kernelDebugSymbolsPath[KDK_PATH_SIZE];
};

char* findKDKWithBuildVersion(const char *basePath, const char *substring)
{
    DIR *dir = opendir(basePath);

    if (!dir)
    {
        printf("Error opening directory");

        return NULL;
    }

    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL)
    {
        if (strstr(entry->d_name, substring))
        {
            char childName[KDK_PATH_SIZE];
            snprintf(childName, KDK_PATH_SIZE, "%s/%s", basePath, entry->d_name);

            printf("Found KDK with build version '%s': %s\n", substring, childName);

            closedir(dir);

            return strdup(childName);
        }
    }

    closedir(dir);

    return NULL;
}

kern_return_t readKDKKernelFromPath(const char *path, char **out_buffer)
{
    int fd = open(path, O_RDONLY);
    
    if(fd == -1)
    {
        printf("Error opening file: %d\n", error);

        *out_buffer = NULL;

        return KERN_FAILURE;
    }
    
    size_t size = lseek(fd, 0, SEEK_END);

    lseek(fd, 0, SEEK_SET);
    
    char *buffer = (char *)malloc(size);
    
    size_t bytes_read = 0;

    bytes_read = read(fd, buffer, size);
    
    close(fd);
    
    *out_buffer = buffer;
    
    return KERN_SUCCESS;
}

char* getKDKKernelNameFromType(KDKKernelType type)
{
    switch(type)
    {
        case KdkKernelTypeRelease:
            return "kernel";
        case KdkKernelTypeReleaseT6000:
            return "kernel.release.t6000";
        case KdkKernelTypeReleaseT6020:
            return "kernel.release.t6020";
        case KdkKernelTypeReleaseT8103:
            return "kernel.release.t8103";
        case KdkKernelTypeReleaseT8112:
            return "kernel.release.t8112";
        case KdkKernelTypeReleaseVmApple:
            return "kernel.release.vmapple";
        case KdkKernelTypeDevelopment:
            return "kernel.development";
        case KdkKernelTypeDevelopmentT6000:
            return "kernel.development.t6000";
        case KdkKernelTypeDevelopmentT6020:
            return "kernel.development.t6020";
        case KdkKernelTypeDevelopmentT8103:
            return "kernel.development.t8103";
        case KdkKernelTypeDevelopmentT8112:
            return "kernel.development.t8112";
        case KdkKernelTypeDevelopmentVmApple:
            return "kernel.development.vmapple";
        case KdkKernelTypeKasan:
            return "kernel.kasan";
        case KdkKernelTypeKasanT6000:
            return "kernel.kasan.t6000";
        case KdkKernelTypeKasanT6020:
            return "kernel.kasan.t6020";
        case KdkKernelTypeKasanT8103:
            return "kernel.kasan.t8103";
        case KdkKernelTypeKasanT8112:
            return "kernel.kasan.t8112";
        case KdkKernelTypeKasanVmApple:
            return "kernel.kasan.vmapple";
        default:
            return "";
    }

    return NULL;
}

void getKDKPathFromBuildInfo(const char *buildVersion, char *outPath)
{
    char* KDK = findKDKWithBuildVersion("/Library/Developer/KDKs", buildVersion);

    if(outPath)
    {
        if(KDK)
        {
            strlcpy(outPath, KDK, KDK_PATH_SIZE);

            delete KDK;
        } else
        {
            *outPath = '\0';
        }
    }
}

void getKDKKernelFromPath(const char *path, const char *kernelVersion, KDKKernelType *outType, char *outKernelPath)
{
    KDKKernelType type = KdkKernelTypeNone;

    if(strstr(kernelVersion, "RELEASE"))
    {
        if(strstr(kernelVersion, "T6000"))
        {
            type = KdkKernelTypeReleaseT6000;
        } else if(strstr(kernelVersion, "T6020"))
        {
            type = KdkKernelTypeReleaseT6020;
        } else if(strstr(kernelVersion, "T8103"))
        {
            type = KdkKernelTypeReleaseT8103;
        } else if(strstr(kernelVersion, "T8112"))
        {
            type = KdkKernelTypeReleaseT8112;
        } else if(strstr(kernelVersion, "VMAPPLE"))
        {
            type = KdkKernelTypeReleaseVmApple;
        } else
        {
            type = KdkKernelTypeRelease;
        }
    }

    if(strstr(kernelVersion, "DEVELOPMENT"))
    {
        if(strstr(kernelVersion, "T6000"))
        {
            type = KdkKernelTypeDevelopmentT6000;
        } else if(strstr(kernelVersion, "T6020"))
        {
            type = KdkKernelTypeDevelopmentT6020;
        } else if(strstr(kernelVersion, "T8103"))
        {
            type = KdkKernelTypeDevelopmentT8103;
        } else if(strstr(kernelVersion, "T8112"))
        {
            type = KdkKernelTypeDevelopmentT8112;
        } else if(strstr(kernelVersion, "VMAPPLE"))
        {
            type = KdkKernelTypeDevelopmentVmApple;
        } else
        {
            type = KdkKernelTypeDevelopment;
        }
    }
    
    if(strstr(kernelVersion, "KASAN"))
    {
        if(strstr(kernelVersion, "T6000"))
        {
            type = KdkKernelTypeKasanT6000;
        } else if(strstr(kernelVersion, "T6020"))
        {
            type = KdkKernelTypeKasanT6020;
        } else if(strstr(kernelVersion, "T8103"))
        {
            type = KdkKernelTypeKasanT8103;
        } else if(strstr(kernelVersion, "T8112"))
        {
            type = KdkKernelTypeKasanT8112;
        } else if(strstr(kernelVersion, "VMAPPLE"))
        {
            type = KdkKernelTypeKasanVmApple;
        } else
        {
            type = KdkKernelTypeKasan;
        }
    }

    if(type == KdkKernelTypeNone)
    {
        *outType = KdkKernelTypeNone;
        *outKernelPath = '\0';

    } else
    {
        *outType = type;

        snprintf(outKernelPath, KDK_PATH_SIZE, "%s/System/Library/Kernels/", path, getKDKKernelNameFromType(type));
    }
}

void KDKFromBuildInfo(const char *buildVersion, const char *kernelVersion)
{
    struct KDKInfo *kdkInfo;

    if(!buildVersion || !kernelVersion)
    {
        if(!buildVersion)
            printf("MacRK::macOS Build Version not found!");

        if(!kernelVersion)
            printf("MacRK::macOS Kernel Version not found!");

        return;
    }

    kdkInfo = new KDKInfo;

    getKDKPathFromBuildInfo(buildVersion, kdkInfo->path);
    getKDKKernelFromPath(kdkInfo->path, kernelVersion, &kdkInfo->type, kdkInfo->kernelPath);

    if(kdkInfo->path[0] == '\0' ||
       kdkInfo->type == KdkKernelTypeNone ||
       kdkInfo->kernelPath[0] == '\0')
    {
        delete kdkInfo;

        printf("MacRK::Failed to find KDK with buildVersion %s and kernelVersion %s", buildVersion, kernelVersion);

        return;
    }

    kdkInfo->kernelName = getKDKKernelNameFromType(kdkInfo->type);

    snprintf(kdkInfo->kernelDebugSymbolsPath, KDK_PATH_SIZE, "%s/System/Library/Kernels/%s.dSYM/Contents/Resources/DWARF/%s", kdkInfo->path, kdkInfo->kernelName, kdkInfo->kernelName);

    printf("kernel:%s debug symbols:%s", kdkInfo->kernelPath, kdkInfo->kernelDebugSymbolsPath);
}

const char* getKernelVersion()
{
    char *kernelBuildVersion = new char[256];

    struct utsname kernelInfo;

    uname(&kernelInfo);

    strlcpy(kernelBuildVersion, kernelInfo.version, 256);

    printf("MacRK::macOS kernel version = %s\n", kernelInfo.version);

    return kernelBuildVersion;
}

const char* getOSBuildVersion()
{
    int mib[2];

    size_t len = 256;
    char *buildVersion = new char[len];

    mib[0] = CTL_KERN;
    mib[1] = KERN_OSVERSION;

    if (sysctl(mib, 2, buildVersion, &len, NULL, 0) == 0)
    {
        printf("MacRK::macOS OS build version = %s\n", buildVersion);
    } else
    {
        return NULL;
    }

    return buildVersion;
}

void writeToFile(char *file_data, size_t file_size)
{
    const char *file_path = NULL;

    file_path = "kernel.release.t6000";

    int fd = open(file_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    
    if (fd == -1)
    {
        exit(-1);
    }

    ssize_t bytes_written = write(fd, file_data, file_size);

    if (bytes_written == -1)
    {
        exit(-1);
    }
}

char* getMachOFromFatHeader(char *file_data)
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

        if(cputype == CPU_TYPE_ARM64)
        {
            return file_data + offset;
        }
    #elif __x86_64__

        if(cputype == CPU_TYPE_ARM64)
        {
            return file_data + offset;
        }

    #endif

        arch++;
    }

    return NULL;
}

void getMappingInfoForMachO(char *file_data, size_t *size, uintptr_t *load_addr)
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

void updateSymbolTableForMappedMachO(char *file_data, uintptr_t newLoadAddress, uintptr_t oldLoadAddress)
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

                    uint8_t type;

                    mach_vm_address_t address;

                    name = &strtab[nl->n_strx];

                    type = nl->n_type;

                    address = (nl->n_value - oldLoadAddress) + newLoadAddress;

                    if((nl->n_value - oldLoadAddress) == 0)
                    {
                        address = 0;
                    }

                    nl->n_value = address;

                    printf("Symbol %s = 0x%llx type = 0x%llx\n", name, address, type);
                }
            }
        }

        q += cmdsize;
    };
}

void updateSegmentLoadCommandsForNewLoadAddress(char *file_data, uintptr_t newLoadAddress, uintptr_t oldLoadAddress)
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

bool mapSegmentsFromMachO(char *file_data)
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
    }

    return true;
}

void loadKernelMachO(const char *kernelPath, uintptr_t *loadAddress, size_t *loadSize, uintptr_t *oldLoadAddress)
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
        file_data = getMachOFromFatHeader(file_data);
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

    updateSymbolTableForMappedMachO(file_data, (uintptr_t) baseAddress, *oldLoadAddress);

    updateSegmentLoadCommandsForNewLoadAddress(file_data, (uintptr_t) baseAddress, *oldLoadAddress);

    success = mapSegmentsFromMachO(file_data);

    if(!success)
    {
        printf("Map Segments failed!\n");

        goto fail;
    }

    *loadAddress = (uintptr_t) baseAddress;

    writeToFile((char*) baseAddress, *loadSize);

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

void loadKernel(const char *kernelPath, off_t slide, bool debugSymbols)
{
    uintptr_t loadAddress = 0;

    uintptr_t oldLoadAddress = 0;

    size_t loadSize = 0;

    loadKernelMachO(kernelPath, &loadAddress, &loadSize, &oldLoadAddress);
}

int main()
{
    // loadKernel("/Library/Developer/KDKs/KDK_13.3.1_22E261.kdk/System/Library/Kernels/kernel.release.t6000", 0, false);

    // printf("Build Version: %s OSBuildVersion: %s\n", getKernelVersion(), getOSBuildVersion());

    KDKFromBuildInfo(getOSBuildVersion(), getKernelVersion());

    // loadKernel("/Library/Developer/KDKs/KDK_13.3.1_22E261.kdk/System/Library/Extensions/apfs.kext/Contents/MacOS/apfs", 0, false);

    // loadKernel("/Library/Developer/KDKs/KDK_13.3.1_22E261.kdk/System/Library/Kernels/kernel.release.t6000.dSYM/Contents/Resources/DWARF/kernel.release.t6000", 0, true);

    /*
    size_t size = 0x4000;

    void* segmentAddress = (void*) (0x7FFFF7004000);

    void* segmentMappedMemory = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);

    printf("mmap() return 0x%llx\n", (uint64_t) segmentMappedMemory);

    if (segmentMappedMemory == MAP_FAILED)
    {
        if (errno == EINVAL) {
            fprintf(stderr, "Invalid argument\n");
        } else if (errno == ENOMEM) {
            fprintf(stderr, "Insufficient memory\n");
        } else {
            fprintf(stderr, "Unknown error: %s\n", strerror(errno));
        }

        printf("mmap(0x%lx, 0x%zx, %d) failed!", (uintptr_t) 0x10000000, size, PROT_READ | PROT_WRITE);

        return -1;
    }

    if (mprotect(segmentMappedMemory, size, PROT_READ | PROT_EXEC) == -1)
    {
        printf("mprotect() failed!\n");

        return -1;
    }
    */

    return 0;
}
