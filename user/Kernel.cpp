#include "Kernel.hpp"

Kernel::Kernel()
{
	this->kernel = this;

	Task();

	this->connection = open_kernel_tfp0_connection();

	// UserMachO *userMachO = new UserMachO();
#ifdef __x86_64__
	//userMachO->initWithFilePath("/System/Library/Kernels/kernel");
#elif __arm64__
	// userMachO->initWithFilePath("/System/Library/Kernels/kernel.release.t8110");
#endif

	 //this->macho = userMachO;

	this->slide = this->getSlide();
}

Kernel::~Kernel()
{
	close_kernel_tfp0_connection();

	delete this->macho;
}

mach_vm_address_t Kernel::getBase()
{
	return get_kernel_base();
}

off_t Kernel::getSlide()
{
	return get_kernel_slide();
}

uint64_t Kernel::call(char *symbolname, uint64_t *arguments, size_t argCount)
{
	return kernel_call_function(symbolname, arguments, argCount);
}

uint64_t Kernel::call(mach_vm_address_t func, uint64_t *arguments, size_t argCount)
{
	return kernel_call(func, arguments, argCount);
}

mach_vm_address_t Kernel::vmAllocate(size_t size)
{
	return kernel_vm_allocate(size);
}

mach_vm_address_t Kernel::vmAllocate(size_t size, uint32_t flags, vm_prot_t prot)
{
	return 0;
}

void Kernel::vmDeallocate(mach_vm_address_t address, size_t size)
{
	kernel_vm_deallocate(address, size);
}

bool Kernel::vmProtect(mach_vm_address_t address, size_t size, vm_prot_t prot)
{
	return kernel_vm_protect(address, size, prot);
}

void* Kernel::vmRemap(mach_vm_address_t address, size_t size)
{
	return kernel_vm_remap(address, size);
}

uint64_t Kernel::virtualToPhysical(mach_vm_address_t address)
{
	return kernel_virtual_to_physical(address);
}

bool Kernel::physicalRead(uint64_t paddr, void *data, size_t size)
{
	return false;
}

uint64_t Kernel::physicalRead64(uint64_t paddr)
{
	return phys_read64(paddr);
}

uint32_t Kernel::physicalRead32(uint64_t paddr)
{
	return phys_read32(paddr);
}

uint16_t Kernel::physicalRead16(uint64_t paddr)
{
	return phys_read16(paddr);
}

uint8_t  Kernel::physicalRead8(uint64_t paddr)
{
	return phys_read64(paddr);
}

bool Kernel::physicalWrite(uint64_t paddr, void *data, size_t size)
{
	return false;
}

void Kernel::physicalWrite64(uint64_t paddr, uint64_t value)
{
	phys_write64(paddr, value);
}

void Kernel::physicalWrite32(uint64_t paddr, uint32_t value)
{
	phys_write32(paddr, value);
}

void Kernel::physicalWrite16(uint64_t paddr, uint16_t value)
{
	phys_write16(paddr, value);
}

void  Kernel::physicalWrite8(uint64_t paddr, uint8_t value)
{
	phys_write8(paddr, value);
}


bool Kernel::read(mach_vm_address_t address, void *data, size_t size)
{
	return kernel_read(address, data, size);
}

bool Kernel::readUnsafe(mach_vm_address_t address, void *data, size_t size)
{
	return false;
}


uint8_t Kernel::read8(mach_vm_address_t address)
{
	return kernel_read8(address);
}

uint16_t Kernel::read16(mach_vm_address_t address)
{
	return kernel_read16(address);
}

uint32_t Kernel::read32(mach_vm_address_t address)
{
	return kernel_read32(address);
}

uint64_t Kernel::read64(mach_vm_address_t address)
{
	return kernel_read64(address);
}


bool Kernel::write(mach_vm_address_t address, void *data, size_t size)
{
	return kernel_write(address, data, size);
}

bool Kernel::writeUnsafe(mach_vm_address_t address, void *data, size_t size)
{
	return false;
}


void Kernel::write8(mach_vm_address_t address, uint8_t value)
{
	kernel_write8(address, value);
}

void Kernel::write16(mach_vm_address_t address, uint16_t value)
{
	kernel_write16(address, value);
}

void Kernel::write32(mach_vm_address_t address, uint32_t value)
{
	kernel_write32(address, value);
}

void Kernel::write64(mach_vm_address_t address, uint64_t value)
{
	kernel_write64(address, value);
}

bool Kernel::hookFunction(char *symname, mach_vm_address_t hook, size_t hook_size)
{
	return false;
}

bool Kernel::hookFunction(mach_vm_address_t address, mach_vm_address_t hook, size_t hook_size)
{
	return false;
}

bool Kernel::setBreakpoint(char *symname)
{
	return false;
}

bool Kernel::setBreakpoint(char *symname, mach_vm_address_t hook, size_t hook_size)
{
	return false;
}

bool Kernel::setBreakpoint(mach_vm_address_t address)
{
	return false;
}

bool Kernel::setBreakpoint(mach_vm_address_t address, mach_vm_address_t breakpoint_hook, size_t breakpoint_hook_size)
{
	return false;
}

#define MAX_LENGTH 0x100

char* Kernel::readString(mach_vm_address_t address)
{
	char *s;

	int index = 0;

	char c;

	do
	{
		c = static_cast<char>(this->read8(address + index));

		index++;
		
	} while(c);

	s = new char[index + 1];

	this->read(address, reinterpret_cast<void*>(s), index);

	s[index] = '\0';

	return s;
}

Symbol* Kernel::getSymbolByName(char *symname)
{
	return NULL;
}

Symbol* Kernel::getSymbolByAddress(mach_vm_address_t address)
{
	return NULL;
}

mach_vm_address_t Kernel::getSymbolAddressByName(char *symbolname)
{
	return get_kernel_symbol(symbolname);
}