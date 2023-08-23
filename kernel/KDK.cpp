#include "Kernel.hpp"
#include "Dwarf.hpp"
#include "MachO.hpp"

#include <sys/param.h>
#include <sys/mount.h>
#include <sys/vnode.h>
#include <sys/proc.h>
#include <sys/kauth.h>

#include <libkern/libkern.h>

using namespace xnu;

char* findKDKWithBuildVersion(const char *basePath, const char *substring)
{
    vnode_t vnode = NULLVP;

    vfs_context_t context = vfs_context_create(NULL);

    int error = vnode_lookup(basePath, 0, &vnode, context);

    vfs_context_rele(context);

    if (error == 0 && vnode)
    {
        struct vnode_attr vattr;

        VATTR_INIT(&vattr);
        VATTR_WANTED(&vattr, va_type);

        vattr.va_type = VDIR;

        vnode_t childVnode = NULLVP;

        int result = VNOP_READDIR(vnode, &childVnode, &vattr, NULL, 0, NULL, vfs_context_kernel());

        while (result == 0 && childVnode != NULLVP)
        {
            char childName[KDK_PATH_SIZE];

            vn_getpath(childVnode, childName, KDK_PATH_SIZE);

            if(strstr(childName, substring))
            {
                MAC_RK_LOG("MacRK::Found KDK with build version '%s': %s\n", substring, childName);

                return strdup(childName);
            }

            vnode_t nextChildVnode = NULLVP;

            result = VNOP_READDIR(vnode, &nextChildVnode, &vattr, NULL, 0, NULL, vfs_context_kernel());

            vnode_put(childVnode);

            childVnode = nextChildVnode;
        }

        vnode_put(vnode);
    }

    return NULL;
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
		*outType = '\0';
		*outKernelPath = '\0';

	} else
	{
		*outType = type;

		snprintf(outKernelPath, KDK_PATH_SIZE, "%s/System/Library/Kernels/", path, getKDKKernelNameFromType(type));
	}
}

KDK* KDK::KDKFromBuildInfo(xnu::Kernel *kernel, const char *buildVersion, const char *kernelVersion)
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

	KDK::getKDKPathFromBuildInfo(buildVersion, &kdkInfo->path);
	KDK::getKDKKernelFromPath(kdkPath, kernelVersion, &kdkInfo->type, &kdkInfo->kernelPath);

	if(kdkInfo->path[0] == '\0' ||
	   kdkInfo->type[0] == '\0' ||
	   kdkInfo->kernelPath[0] == '\0')
	{
		delete kdkInfo;

		MAC_RK_LOG("MacRK::Failed to find KDK with buildVersion %s and kernelVersion %s", buildVersion, kernelVersion);

		return NULL;
	}

	kdkInfo->kernelName = getKDKKernelNameFromType(kdkInfo->type);

	snprintf(&kdkInfo->kernelDebugSymbolsPath, KDK_PATH_SIZE, "%s/System/Library/Kernels/%s.dSYM/Contents/Resources/DWARF/%s", kdkInfo->path, kdkInfo->kernelName, kdkInfo->kernelName);

	return new KDK(kernel, kdkInfo);
}

KDK::KDK(xnu::Kernel *kernel, struct KDKInfo *kdkInfo)
{
	this->kernel = kernel;
	this->kdkInfo = kdkInfo;
	this->type = kdkInfo->type;
	this->path = &kdkInfo->path;
}

mach_vm_address_t KDK::getKDKSymbolAddressByName(const char *sym)
{

}

Symbol* KDK::getKDKSymbolByName(char *symname)
{

}

Symbol* KDK::getKDKSymbolByAddress(mach_vm_address_t address)
{

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