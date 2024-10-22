/*
 * Copyright (c) YungRaj
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "payload.h"

#include "hook.h"

using namespace darwin;
using namespace xnu;

bool Payload::ReadBytes(UInt8* bytes, Size size) {
    bool success;

    success = ReadBytes(current_offset, bytes, size);

    return success;
}

bool Payload::ReadBytes(Offset offset, UInt8* bytes, Size size) {
    bool success;

    xnu::mach::VmAddress address = address + offset;

    success = GetTask()->Read(address + offset, (void*)bytes, size);

    return success;
}

bool Payload::WriteBytes(UInt8* bytes, Size size) {
    bool success;

    success = WriteBytes(current_offset, bytes, size);

    if (success)
        current_offset += size;

    return success;
}

bool Payload::WriteBytes(Offset offset, UInt8* bytes, Size size) {
    bool success;

    xnu::mach::VmAddress address = address + offset;

    success = GetTask()->Write(address, (void*)bytes, size);

#ifdef __KERNEL__

    if (address >= (xnu::mach::VmAddress)Kernel::GetExecutableMemory() &&
        address < (xnu::mach::VmAddress)Kernel::GetExecutableMemory() +
                      Kernel::GetExecutableMemorySize()) {
        Kernel::SetExecutableMemoryOffset(Kernel::GetExecutableMemoryOffset() + size);
    }

#endif

    return success;
}

bool Payload::Prepare() {
    bool success;

    xnu::mach::VmAddress trampoline;

    Task* task = GetTask();

#if defined(__x86_64__) || (defined(__arm64__) && defined(__USER__))

    trampoline =
        task->VmAllocate(Payload::expectedSize, VM_FLAGS_ANYWHERE, VM_PROT_READ | VM_PROT_EXECUTE);

    if (!trampoline)
        return false;

/*#elif defined(__arm64__) && defined(__KERNEL__)*/
#else

    trampoline = Kernel::GetExecutableMemory() + Kernel::GetExecutableMemoryOffset();

#endif

    address = trampoline;

    return true;
}

void Payload::SetWritable() {
    task->VmProtect(address, Payload::expectedSize, VM_PROT_READ | VM_PROT_WRITE);
}

void Payload::SetExecutable() {
    task->VmProtect(address, Payload::expectedSize, VM_PROT_READ | VM_PROT_EXECUTE);
}

bool Payload::Commit() {
#if defined(__x86_64__) || (defined(__arm64__) && defined(__USER__))
    SetExecutable();
#endif
    return true;
}
