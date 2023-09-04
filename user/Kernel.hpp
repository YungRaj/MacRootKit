#ifndef __KERNEL_HPP_
#define __KERNEL_HPP_

#include "Task.hpp"

#include "MachO.hpp"
#include "UserMachO.hpp"

#include "Disassembler.hpp"

extern "C"
{
	#include "kern_user.h"
}

#include <mach/mach_types.h>

#include <sys/sysctl.h>
#include <sys/utsname.h>

class MachO;
class Symbol;

namespace xnu
{
	class Task;

	const char* getKernelVersion();
	const char* getOSBuildVersion();
	
	class Kernel : public xnu::Task
	{
		public:
			Kernel();

			~Kernel();

			virtual mach_vm_address_t getBase();

			virtual off_t getSlide();

			virtual uint64_t call(char *symbolname, uint64_t *arguments, size_t argCount);
			virtual uint64_t call(mach_vm_address_t func, uint64_t *arguments, size_t argCount);

			virtual mach_vm_address_t vmAllocate(size_t size);
			virtual mach_vm_address_t vmAllocate(size_t size, uint32_t flags, vm_prot_t prot);

			virtual void vmDeallocate(mach_vm_address_t address, size_t size);

			virtual bool vmProtect(mach_vm_address_t address, size_t size, vm_prot_t prot);

			virtual void* vmRemap(mach_vm_address_t address, size_t size);

			virtual uint64_t virtualToPhysical(mach_vm_address_t address);

			virtual bool physicalRead(uint64_t paddr, void *data, size_t size);

			virtual uint64_t physicalRead64(uint64_t paddr);
			virtual uint32_t physicalRead32(uint64_t paddr);
			virtual uint16_t physicalRead16(uint64_t paddr);
			virtual uint8_t  physicalRead8(uint64_t paddr);

			virtual bool physicalWrite(uint64_t paddr, void *data, size_t size);

			virtual void physicalWrite64(uint64_t paddr, uint64_t value);
			virtual void physicalWrite32(uint64_t paddr, uint32_t value);
			virtual void physicalWrite16(uint64_t paddr, uint16_t value);
			virtual void  physicalWrite8(uint64_t paddr, uint8_t value);

			virtual bool read(mach_vm_address_t address, void *data, size_t size);
			virtual bool readUnsafe(mach_vm_address_t address, void *data, size_t size);

			virtual uint8_t read8(mach_vm_address_t address);
			virtual uint16_t read16(mach_vm_address_t address);
			virtual uint32_t read32(mach_vm_address_t address);
			virtual uint64_t read64(mach_vm_address_t address);

			virtual bool write(mach_vm_address_t address, void *data, size_t size);
			virtual bool writeUnsafe(mach_vm_address_t address, void *data, size_t size);

			virtual void write8(mach_vm_address_t address, uint8_t value);
			virtual void write16(mach_vm_address_t address, uint16_t value);
			virtual void write32(mach_vm_address_t address, uint32_t value);
			virtual void write64(mach_vm_address_t address, uint64_t value);

			virtual bool hookFunction(char *symname, mach_vm_address_t hook, size_t hook_size);
			virtual bool hookFunction(mach_vm_address_t address, mach_vm_address_t hook, size_t hook_size);

			virtual bool setBreakpoint(char *symname);
			virtual bool setBreakpoint(char *symname, mach_vm_address_t hook, size_t hook_size);

			virtual bool setBreakpoint(mach_vm_address_t address);
			virtual bool setBreakpoint(mach_vm_address_t address, mach_vm_address_t breakpoint_hook, size_t breakpoint_hook_size);

			virtual char* readString(mach_vm_address_t address);

			virtual Symbol* getSymbolByName(char *symname);
			virtual Symbol* getSymbolByAddress(mach_vm_address_t address);

			virtual mach_vm_address_t getSymbolAddressByName(char *symbolname);

		private:
			mrk::UserMachO *macho;

			mach_port_t connection;

			Disassembler *disassembler;

			mach_vm_address_t base;

			off_t slide;

	};

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

	class KDK
	{
		public:
			explicit KDK(xnu::Kernel *kernel, struct KDKInfo *kdkInfo);

			static KDK* KDKFromBuildInfo(xnu::Kernel *kernel, const char *buildVersion, const char *kernelVersion);
			static KDKInfo* KDKInfoFromBuildInfo(xnu::Kernel *kernel, const char *buildVersion, const char *kernelVersion);

			char* getPath() { return path; }

			xnu::Kernel* getKernel() { return kernel; }

			Debug::Dwarf* getDwarf() { return dwarf; }

			MachO* getMachO() { return dynamic_cast<MachO*>(kernelWithDebugSymbols);  }

			mach_vm_address_t getBase() { return base; }

			mach_vm_address_t getKDKSymbolAddressByName(const char *sym);

			Symbol* getKDKSymbolByName(char *symname);
			Symbol* getKDKSymbolByAddress(mach_vm_address_t address);

			char* findString(char *s);

			std::Array<mach_vm_address_t> getExternalReferences(mach_vm_address_t addr);

			std::Array<mach_vm_address_t> getStringReferences(mach_vm_address_t addr);
			std::Array<mach_vm_address_t> getStringReferences(const char *s);

			void parseDebugInformation();

		private:
			bool valid;

			const char *path;

			KDKKernelType type;

			xnu::Kernel *kernel;

			xnu::KernelMachO *kernelWithDebugSymbols;

			Debug::Dwarf *dwarf;

			mach_vm_address_t base;
	};
};

#endif