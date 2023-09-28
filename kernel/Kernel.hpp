#ifndef __KERNEL_HPP_
#define __KERNEL_HPP_

extern "C"
{
	#include <libkern/libkern.h>

	#include <kern/host.h>

	#include <mach/mach_types.h>

	#include <sys/sysctl.h>
}

#include <IOKit/IOLib.h>

#include "MacRootKit.hpp"

#include "IOKernelRootKitService.hpp"

#include "Task.hpp"

#include "Disassembler.hpp"
#include "Dwarf.hpp"

#include "KernelMachO.hpp"

class IOKernelRootKitService;

class MachO;
class Symbol;

class Disassembler;

namespace mrk
{
	class MacRootKit;
};

namespace Debug
{
	class Dwarf;
};

namespace xnu
{
	class KDK;

	static const char* getKernelVersion();

	static const char* getOSBuildVersion();

	class Kernel : public xnu::Task
	{
		static constexpr size_t tempExecutableMemorySize {4096 * 4 * 32};

		static off_t tempExecutableMemoryOffset;

		static uint8_t tempExecutableMemory[tempExecutableMemorySize];

		static xnu::Kernel *kernel;

		public:
			Kernel(mach_port_t kernel_task_port);

			Kernel(mach_vm_address_t cache, mach_vm_address_t base, off_t slide);

			Kernel(mach_vm_address_t base, off_t slide);

			~Kernel();

			static xnu::Kernel* create(mach_port_t kernel_task_port);

			static xnu::Kernel* create(mach_vm_address_t cache, mach_vm_address_t base, off_t slide);

			static xnu::Kernel* create(mach_vm_address_t base, off_t slide);

			static mach_vm_address_t findKernelCache();

			static mach_vm_address_t findKernelCollection();

			static mach_vm_address_t findKernelBase();

			static off_t findKernelSlide();

			static mach_vm_address_t getExecutableMemory() { return reinterpret_cast<mach_vm_address_t>(&tempExecutableMemory[0]); }

			static mach_vm_address_t getExecutableMemoryAtOffset(off_t offset) { return reinterpret_cast<mach_vm_address_t>(&tempExecutableMemory[tempExecutableMemoryOffset]); }

			static size_t getExecutableMemorySize() { return tempExecutableMemorySize; }

			static off_t getExecutableMemoryOffset() { return tempExecutableMemoryOffset; }

			static void setExecutableMemoryOffset(off_t offset) { tempExecutableMemoryOffset = offset; }

			const char* getVersion() const { return version; }

			const char* getOSBuildVersion() const { return osBuildVersion; }

			MachO* getMachO() const { return macho; }

			virtual mach_vm_address_t getBase();

			virtual off_t getSlide();

			void setRootKit(mrk::MacRootKit *rootkit) { this->rootkit = rootkit; }

			mrk::MacRootKit* getRootKit() { return this->rootkit; }

			void setRootKitService(IOKernelRootKitService *service) { this->rootkitService = service; }

			IOKernelRootKitService* getRootKitService() { return this->rootkitService; }

			mach_port_t getKernelTaskPort() { return this->kernel_task_port; }

			bool setKernelWriting(bool enable);

			bool setNXBit(bool enable);

			bool setInterrupts(bool enable);

			void getKernelObjects();

			virtual task_t getKernelTask() { return this->getTask(); }

			virtual vm_map_t getKernelMap() { return this->getMap(); }

			virtual pmap_t getKernelPmap() { return this->getPmap(); }

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

			virtual char* readString(mach_vm_address_t address);

			virtual Symbol* getSymbolByName(char *symbolname);
			virtual Symbol* getSymbolByAddress(mach_vm_address_t address);

			virtual mach_vm_address_t getSymbolAddressByName(char *symbolname);

		protected:
			KDK *kernelDebugKit;

			MachO *macho;

			IOKernelRootKitService *rootkitService;

			mrk::MacRootKit *rootkit;

			mach_port_t kernel_task_port;

		private:
			const char *version;
			const char *osBuildVersion;

			IOSimpleLock *kernelWriteLock;

			void createKernelTaskPort();
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

	template<typename T>
	struct Xref
	{
		mach_vm_address_t what;

		mach_vm_address_t where;

		T data;
	};

	class KDK
	{
		public:
			explicit KDK(xnu::Kernel *kernel, struct KDKInfo *kdkInfo);

			static KDK* KDKFromBuildInfo(xnu::Kernel *kernel, const char *buildVersion, const char *kernelVersion);
			static KDKInfo* KDKInfoFromBuildInfo(xnu::Kernel *kernel, const char *buildVersion, const char *kernelVersion);

			static void getKDKPathFromBuildInfo(const char *buildVersion, char *outPath);

			static void getKDKKernelFromPath(const char *path, const char *kernelVersion, KDKKernelType *outType, char *outKernelPath);

			xnu::Kernel* getKernel() const { return kernel; }

			Debug::Dwarf* getDwarf() const { return dwarf; }

			MachO* getMachO() const { return dynamic_cast<MachO*>(kernelWithDebugSymbols);  }

			mach_vm_address_t getBase() const { return base; }

			char* getPath() const { return path; }

			mach_vm_address_t getKDKSymbolAddressByName(const char *sym);

			Symbol* getKDKSymbolByName(char *symname);
			Symbol* getKDKSymbolByAddress(mach_vm_address_t address);

			char* findString(char *s);

			template<typename T>
			std::Array<Xref<T>*> getExternalReferences(mach_vm_address_t addr);

			template<typename T>
			std::Array<Xref<T>*> getStringReferences(mach_vm_address_t addr);

			template<typename T>
			std::Array<Xref<T>*> getStringReferences(const char *s);

			void parseDebugInformation();

		private:
			bool valid;

			char *path;

			KDKInfo *kdkInfo;

			KDKKernelType type;

			xnu::Kernel *kernel;

			xnu::KernelMachO *kernelWithDebugSymbols;

			Debug::Dwarf *dwarf;

			mach_vm_address_t base;
	};
};

#endif