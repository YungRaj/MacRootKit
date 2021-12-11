#include "Kernel.hpp"

Kernel::Kernel()
{
}

Kernel::~Kernel()
{
}

mach_vm_address_t Kernel::getBase()
{
}

off_t Kernel::getSlide()
{
}

uint64_t Kernel::call(char *symbolname, uint64_t *arguments, size_t argCount)
{
}

uint64_t Kernel::call(mach_vm_address_t func, uint64_t *arguments, size_t argCount)
{
}

mach_vm_address_t Kernel::vmAllocate(size_t size)
{
}

mach_vm_address_t Kernel::vmAllocate(size_t size, uint32_t flags, vm_prot_t prot)
{
}

void Kernel::vmDeallocate(mach_vm_address_t address, size_t size)
{
}


bool Kernel::vmProtect(mach_vm_address_t address, size_t size, vm_prot_t prot)
{
}

void* Kernel::vmRemap(mach_vm_address_t address, size_t size)
{
}

uint64_t Kernel::virtualToPhysical(mach_vm_address_t address)
{
}

bool Kernel::physicalRead(uint64_t paddr, void *data, size_t size)
{
}

uint64_t Kernel::physicalRead64(uint64_t paddr)
{
}

uint32_t Kernel::physicalRead32(uint64_t paddr)
{
}

uint16_t Kernel::physicalRead16(uint64_t paddr)
{
}

uint8_t  Kernel::physicalRead8(uint64_t paddr)
{
}

bool Kernel::physicalWrite(uint64_t paddr, void *data, size_t size)
{
}

void Kernel::physicalWrite64(uint64_t paddr, uint64_t value)
{
}

void Kernel::physicalWrite32(uint64_t paddr, uint32_t value)
{
}

void Kernel::physicalWrite16(uint64_t paddr, uint16_t value)
{
}

void  Kernel::physicalWrite8(uint64_t paddr, uint8_t value)
{
}


bool Kernel::read(mach_vm_address_t address, void *data, size_t size)
{
}

bool Kernel::readUnsafe(mach_vm_address_t address, void *data, size_t size)
{
}


uint8_t Kernel::read8(mach_vm_address_t address)
{
}

uint16_t Kernel::read16(mach_vm_address_t address)
{
}

uint32_t Kernel::read32(mach_vm_address_t address)
{
}

uint64_t Kernel::read64(mach_vm_address_t address)
{
}


bool Kernel::write(mach_vm_address_t address, void *data, size_t size)
{
}

bool Kernel::writeUnsafe(mach_vm_address_t address, void *data, size_t size)
{
}


void Kernel::write8(mach_vm_address_t address, uint8_t value)
{
}

void Kernel::write16(mach_vm_address_t address, uint16_t value)
{
}

void Kernel::write32(mach_vm_address_t address, uint32_t value)
{
}

void Kernel::write64(mach_vm_address_t address, uint64_t value)
{
}

bool Kernel::hookFunction(char *symname, mach_vm_address_t hook, size_t hook_size)
{
}

bool hookFunction(mach_vm_address_t address, mach_vm_address_t hook, size_t hook_size)
{
}

bool Kernel::setBreakpoint(char *symname)
{
}

bool setBreakpoint(char *symname, mach_vm_address_t hook, size_t hook_size)
{
}

bool Kernel::setBreakpoint(mach_vm_address_t address)
{
}

bool Kernel::setBreakpoint(mach_vm_address_t address, mach_vm_address_t breakpoint_hook, size_t breakpoint_hook_size)
{
}

char* Kernel::readString(mach_vm_address_t address)
{
}

Symbol* Kernel::getSymbolByName(char *symname)
{
}

Symbol* Kernel::getSymbolByAddress(mach_vm_address_t address)
{
}

mach_vm_address_t Kernel::getSymbolAddressByName(char *symbolname)
{

}