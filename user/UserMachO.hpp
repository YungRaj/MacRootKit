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

#pragma once

#include <Types.h>

#include <vector>

#include "MachO.hpp"
#include "SymbolTable.hpp"

#include "ObjC.hpp"
#include "Swift.hpp"

#include "Dyld.hpp"

extern "C" {
#include <mach-o.h>
}

class Segment;
class Section;

namespace dyld {
class Dyld;
class Library;
}; // namespace dyld

namespace ObjectiveC {
class ObjCData;
};

namespace xnu {
class Task;
};

namespace mrk {
class CodeSignature {
public:
    explicit CodeSignature(UserMachO* macho, struct linkedit_data_command* cmd)
        : macho(macho), cmd(cmd) {
        this->parseCodeSignature();
    }

    static CodeSignature* codeSignatureWithLinkedit(UserMachO* macho,
                                                    struct linkedit_data_command* cmd);

    UserMachO* getMachO() {
        return macho;
    }

    struct linkedit_data_command* getLoadCommand() {
        return cmd;
    }

    SuperBlob* getSuperBlob() {
        return superBlob;
    }

    code_directory_t getCodeDirectory() {
        return codeDirectory;
    }

    char* getEntitlements() {
        return entitlements;
    }

    bool verifyCodeSlot(UInt8* blob, Size size, bool sha256, char* signature, Size sigsize);

    bool compareHash(UInt8* hash1, UInt8* hash2, Size hashSize);

    UInt8* computeHash(bool sha256, UInt8* blob, Size size);

    bool parseCodeSignature();

private:
    UserMachO* macho;

    struct linkedit_data_command* cmd;

    SuperBlob* superBlob;

    code_directory_t codeDirectory;

    char* entitlements;
};

class UserMachO : public MachO {
public:
    explicit UserMachO() : task(NULL), file_path(NULL) {}
    explicit UserMachO(const char* path);

    ~UserMachO() {}

    virtual void withTask(xnu::Task* task);
    virtual void withFilePath(const char* path);

    virtual void withBuffer(char* buffer);
    virtual void withBuffer(char* buffer, Offset slide);
    virtual void withBuffer(char* buffer, UInt64 size);

    virtual void withBuffer(xnu::Mach::VmAddress base, char* buffer, Offset slide);
    virtual void withBuffer(xnu::Mach::VmAddress base, char* buffer, Offset slide,
                            bool is_dyld_cache);

    virtual void withBuffer(mrk::UserMachO* libobjc, xnu::Mach::VmAddress base, char* buffer,
                            Offset slide);

    char* getFilePath() {
        return this->dyld ? this->dyld->getMainImagePath() : this->file_path;
    }

    bool isDyldCache() {
        return is_dyldCache;
    }

    void setIsDyldCache(bool isDyldCache) {
        this->is_dyldCache = isDyldCache;
    }

    UserMachO* getObjectiveCLibrary() {
        return libobjc;
    }

    ObjectiveC::ObjCData* getObjCMetadata() {
        return objc;
    }

    bool isObjectiveCLibrary() {
        return is_libobjc;
    }

    void setIsObjectiveCLibrary(bool is_libobjc) {
        this->is_libobjc = is_libobjc;
    }

    void setObjectiveCLibrary(UserMachO* libobjc) {
        this->libobjc = libobjc;
    }

    static MachO* taskAt(xnu::Mach::Port task);
    static MachO* libraryLoadedAt(xnu::Mach::Port task, char* library);

    static UInt64 untagPacPointer(xnu::Mach::VmAddress base, enum dyld_fixup_t fixupKind,
                                  UInt64 ptr, bool* bind, bool* auth, UInt16* pac, Size* skip);

    bool isPointerInPacFixupChain(xnu::Mach::VmAddress ptr);

    xnu::Mach::VmAddress getBufferAddress(xnu::Mach::VmAddress address);

    virtual void parseMachO() override;

    virtual void parseHeader() override;

    virtual void parseFatHeader() override;

    virtual void parseSymbolTable(struct nlist_64* symtab, UInt32 nsyms, char* strtab,
                                  Size strsize) override;

    virtual void parseLinkedit() override;

    virtual bool parseLoadCommands() override;

    void parseCodeSignature(struct linkedit_data_command* cmd) {
        codeSignature = CodeSignature::codeSignatureWithLinkedit(this, cmd);
    }

    void parseObjC() {
        this->objc = ObjectiveC::parseObjectiveC(this);
    }

    void parseSwift() {
        this->swift = Swift::parseSwift(this);
    }

    UInt8* operator[](UInt64 index) {
        return this->getOffset(index);
    }

private:
    xnu::Task* task;

    mrk::UserMachO* libobjc;

    dyld::Dyld* dyld;

    xnu::Mach::VmAddress dyld_base;
    xnu::Mach::VmAddress dyld_shared_cache;

    mrk::CodeSignature* codeSignature;

    ObjectiveC::ObjCData* objc;
    Swift::SwiftMetadata* swift;

    char* file_path;

    bool is_dyldCache;
    bool is_libobjc;

    UInt64 readUleb128(UInt8* start, UInt8* end, UInt32* idx);
    Int64 readSleb128(UInt8* start, UInt8* end, UInt32* idx);
};
} // namespace mrk
