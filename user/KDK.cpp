#include "Kernel.hpp"
#include "Dwarf.hpp"
#include "MachO.hpp"
#include "KernelMachO.hpp"

#include <sys/param.h>
#include <sys/mount.h>
#include <sys/vnode.h>
#include <sys/proc.h>
#include <sys/kauth.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/file.h>

#include <libkern/libkern.h>

using namespace xnu;

char* findKDKWithBuildVersion(const char *basePath, const char *substring);

kern_return_t readKDKKernelFromPath(const char *path, char **out_buffer);

class KDKKernelMachO : KernelMachO
{
	public:
		KDKKernelMachO(xnu::Kernel *kernel, const char *path)
		{
			this->kernel = kernel;
			this->path = path;
			this->aslr_slide = kernel->getSlide();

			readKDKKernelFromPath(path, &this->buffer);

			if(!this->buffer)
				panic("MacRK::KDK could not be read from disk at path %s\n", path);

			this->header = reinterpret_cast<struct mach_header_64*>(buffer);
			this->symbolTable = new SymbolTable();

			this->base = this->getBase();
			
			this->parseMachO();
		}

		mach_vm_address_t getBase()
		{
			struct mach_header_64 *hdr = this->header;

			uint8_t *cmds = reinterpret_cast<uint8_t*>(hdr)+ sizeof(struct mach_header_64);

			uint8_t *q = cmds;

			mach_vm_address_t base = UINT64_MAX;

			for(int i = 0; i < hdr->ncmds; i++)
			{
				struct load_command *load_cmd = reinterpret_cast<struct load_command*>(q);

				uint32_t cmdtype = load_cmd->cmd;
				uint32_t cmdsize = load_cmd->cmdsize;

				if(load_cmd->cmd == LC_SEGMENT_64)
				{
					struct segment_command_64 *segment = reinterpret_cast<struct segment_command_64*>(q);

					uint64_t vmaddr = segment->vmaddr;
					uint64_t vmsize = segment->vmsize;

					uint64_t fileoffset = segment->fileoff;
					uint64_t filesize = segment->filesize;

					if(vmaddr < base)
						base = vmaddr;
					
				}

				q = q + load_cmd->cmdsize;
			}

			if(base == UINT64_MAX)
				return 0;

			return base;
		}

		void parseSymbolTable(struct nlist_64 *symtab, uint32_t nsyms, char *strtab, size_t strsize)
		{
			for(int i = 0; i < nsyms; i++)
			{
				Symbol *symbol;

				struct nlist_64 *nl = &symtab[i];

				char *name;

				mach_vm_address_t address;

				name = &strtab[nl->n_strx];

				address = nl->n_value + this->kernel->getSlide();
				// add the kernel slide so that the address is correct

				symbol = new Symbol(this, nl->n_type & N_TYPE, name, address, this->addressToOffset(address), this->segmentForAddress(address), this->sectionForAddress(address));

				this->symbolTable->addSymbol(symbol);
			}

			MAC_RK_LOG("MacRK::KDKKernelMachO::%u syms!\n", nsyms);
		}
	private:
		const char *path;

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

void KDK::getKDKPathFromBuildInfo(const char *buildVersion, char *outPath)
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

void KDK::getKDKKernelFromPath(const char *path, const char *kernelVersion, KDKKernelType *outType, char *outKernelPath)
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

KDK* KDK::KDKFromBuildInfo(xnu::Kernel *kernel, const char *buildVersion, const char *kernelVersion)
{
	return new KDK(kernel, KDK::KDKInfoFromBuildInfo(kernel, buildVersion, kernelVersion));
}

KDKInfo* KDK::KDKInfoFromBuildInfo(xnu::Kernel *kernel, const char *buildVersion, const char *kernelVersion)
{
	struct KDKInfo *kdkInfo;

	if(!buildVersion || !kernelVersion)
	{
		if(!buildVersion)
			MAC_RK_LOG("MacRK::macOS Build Version not found!");

		if(!kernelVersion)
			MAC_RK_LOG("MacRK::macOS Kernel Version not found!");

		return NULL;
	}

	kdkInfo = new KDKInfo;

	KDK::getKDKPathFromBuildInfo(buildVersion, kdkInfo->path);
	KDK::getKDKKernelFromPath(kdkPath, kernelVersion, &kdkInfo->type, kdkInfo->kernelPath);

	if(kdkInfo->path[0] == '\0' ||
	   kdkInfo->type == KdkKernelTypeNone ||
	   kdkInfo->kernelPath[0] == '\0')
	{
		delete kdkInfo;

		MAC_RK_LOG("MacRK::Failed to find KDK with buildVersion %s and kernelVersion %s", buildVersion, kernelVersion);

		return NULL;
	}

	kdkInfo->kernelName = getKDKKernelNameFromType(kdkInfo->type);

	snprintf(kdkInfo->kernelDebugSymbolsPath, KDK_PATH_SIZE, "%s/System/Library/Kernels/%s.dSYM/Contents/Resources/DWARF/%s", kdkInfo->path, kdkInfo->kernelName, kdkInfo->kernelName);

	return kdkInfo;
}

KDK::KDK(xnu::Kernel *kernel, struct KDKInfo *kdkInfo)
{
	this->kernel = kernel;
	this->kdkInfo = kdkInfo;
	this->type = kdkInfo->type;
	this->path = &kdkInfo->path;
	this->kernelWithDebugSymbols = new KDKKernelMachO(kernel, &kdkInfo->kernelDebugSymbolsPath);
}

mach_vm_address_t KDK::getKDKSymbolAddressByName(const char *sym)
{
	return this->kernelWithDebugSymbols->getSymbolAddressByName(sym);
}

Symbol* KDK::getKDKSymbolByName(char *symname)
{
	return this->kernelWithDebugSymbols->getSymbolByName(symname);
}

Symbol* KDK::getKDKSymbolByAddress(mach_vm_address_t address)
{
	return this->kernelWithDebugSymbols->getSymbolByAddress(symname);
}

char* KDK::findString(char *s)
{

}

std::Array<mach_vm_address_t> KDK::getExternalReferences(mach_vm_address_t addr)
{

}

std::Array<mach_vm_address_t> KDK::getStringReferences(mach_vm_address_t addr)
{

}

std::Array<mach_vm_address_t> KDK::getStringReferences(const char *s)
{

}

void KDK::parseDebugInformation()
{

}