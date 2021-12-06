#include "Disassembler.hpp"

Disassembler::Disassembler(Task *task)
{
}

void Disassembler::initDisassembler()
{
}

void Disassembler::deinitDisassembler()
{
}

size_t Disassembler::disassemble(mach_vm_address_t address, size_t size, cs_insn **result)
{
}

size_t Disassembler::quickInstructionSize(mach_vm_address_t address, size_t min)
{
}

size_t Disassembler::instructionSize(mach_vm_address_t address, size_t min)
{
}

mach_vm_address_t Disassembler::disassembleNthCall(mach_vm_address_t address, size_t num, size_t lookup_size)
{
}

mach_vm_address_t Disassembler::disassembleNthJmp(mach_vm_address_t address, size_t num, size_t lookup_size)
{
}

mach_vm_address_t Disassembler::disassembleNthInstruction(mach_vm_address_t, size_t num, size_t lookup_size)
{
}

mach_vm_address_t Disassembler::disassembleSignature(mach_vm_address_t address, Array<DisasmSig*> *signature, size_t num, size_t lookup_size)
{
}