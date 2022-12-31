#ifndef __KERNEL_HPP_
#define __KERNEL_HPP_

#include <IOKit/IOLib.h>

#include <kern/host.h>
#include <mach/mach_types.h>

#include "MacRootKit.hpp"

#include "IOKernelRootKitService.hpp"

#include "Task.hpp"

#include "Disassembler.hpp"

#include "Dwarf.hpp"
#include "KernelMachO.hpp"
#include "Symbol.hpp"

class IOKernelRootKitService;

class MachO;
class Symbol;

class Disassembler;

namespace mrk
{
	class MacRootKit;
};

using namespace mrk;

namespace xnu
{
	class Kernel : public Task
	{
		static constexpr size_t tempExecutableMemorySize {4096 * 4 * 32};

		static off_t tempExecutableMemoryOffset;

		static uint8_t tempExecutableMemory[tempExecutableMemorySize];

		static Kernel *kernel;

		public:
			static Kernel* create(mach_port_t kernel_task_port);

			static Kernel* create(mach_vm_address_t cache, mach_vm_address_t base, off_t slide);

			static Kernel* create(mach_vm_address_t base, off_t slide);

			static mach_vm_address_t findKernelCache();

			static mach_vm_address_t findKernelCollection();

			static mach_vm_address_t findKernelBase();

			static off_t findKernelSlide();

			static mach_vm_address_t getExecutableMemory() { return reinterpret_cast<mach_vm_address_t>(&tempExecutableMemory[0]); }

			static mach_vm_address_t getExecutableMemoryAtOffset(off_t offset) { return reinterpret_cast<mach_vm_address_t>(&tempExecutableMemory[tempExecutableMemoryOffset]); }

			static size_t getExecutableMemorySize() { return tempExecutableMemorySize; }

			static off_t getExecutableMemoryOffset() { return tempExecutableMemoryOffset; }

			static void setExecutableMemoryOffset(off_t offset) { tempExecutableMemoryOffset = offset; }

			~Kernel();

			MachO* getMachO() { return macho; }

			virtual mach_vm_address_t getBase();

			virtual off_t getSlide();

			void setRootKit(MacRootKit *rootkit) { this->rootkit = rootkit; }

			MacRootKit* getRootKit() { return this->rootkit; }

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
			MachO *macho;

			IOKernelRootKitService *rootkitService;

			MacRootKit *rootkit;

			mach_port_t kernel_task_port;

		private:
			IOSimpleLock *kernelWriteLock;

			Kernel(mach_port_t kernel_task_port);

			Kernel(mach_vm_address_t cache, mach_vm_address_t base, off_t slide);

			Kernel(mach_vm_address_t base, off_t slide);

			void createKernelTaskPort();
	};

	using namespace Debug;

	class KDKKernel
	{
		public:
			static KDKKernel* KDKKernelFromFilePath(Kernel *kernel, const char *path);

			explicit KDKKernel(Kernel *kernel, const char *path);

			char* getPath();

			Kernel* getKernel();

			Dwarf* getDwarf();

			MachO* getKernelDebugKitKernel();

			mach_vm_address_t getBase();

			mach_vm_address_t findSymbolByName(Kernel *kernel, const char *sym);

			Symbol* getKDKSymbolByName(char *symname);
			Symbol* getKDKSymbolByAddress(mach_vm_address_t address);

			Symbol* matchSymbolWithKDK(Symbol *s);
			Symbol* matchSymbolWithKDK(mach_vm_address_t address);

			mach_vm_address_t matchAddressWithKDK(mach_vm_address_t addr);

			char* findString(char *s);

			Array<mach_vm_address_t> getExternalReferences(mach_vm_address_t addr);

			Array<mach_vm_address_t> getStringReferences(mach_vm_address_t addr);
			Array<mach_vm_address_t> getStringReferences(const char *s);

			void parseDebugInformation();

		private:
			const char *path;

			Kernel *kernel;

			MachO *kdk;

			Dwarf *dwarf;

			mach_vm_address_t base;
	};
};

#endif