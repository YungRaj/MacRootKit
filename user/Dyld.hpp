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

extern "C" {
#include <mach-o.h>

#include <mach/mach_types.h>
#include <sys/types.h>

#include <dyld_cache_format.h>
}

#include <vector>

#include <Types.h>

class MachO;
class Segment;
class Section;

namespace xnu {
class Task;
class Kernel;
}; // namespace xnu

#define MH_DYLIB_IN_CACHE 0x80000000

namespace dyld {
class Library;

class Dyld {
public:
    explicit Dyld(xnu::Kernel* kernel, xnu::Task* task);

    ~Dyld();

    char* getMainImagePath() {
        return main_image_path;
    }

    xnu::Task* getTask() {
        return task;
    }

    xnu::Mach::VmAddress getMainImageLoadBase() {
        return main_image_load_base;
    }
    xnu::Mach::VmAddress getAllImageInfoAddr() {
        return all_image_info_addr;
    }

    xnu::Mach::VmAddress getDyld() {
        return dyld;
    }
    xnu::Mach::VmAddress getDyldSharedCache() {
        return dyld_shared_cache;
    }

    struct dyld_image_info* getMainImageInfo() {
        return main_image_info;
    }

    Offset getSlide() {
        return slide;
    }

    void getImageInfos();

    void iterateAllImages();

    struct dyld_cache_header* cacheGetHeader();

    struct dyld_cache_mapping_info* cacheGetMappings(struct dyld_cache_header* cache_header);
    struct dyld_cache_mapping_info* cacheGetMapping(struct dyld_cache_header* cache_header,
                                                    xnu::Mach::VmProtection prot);

    void cacheOffsetToAddress(UInt64 dyld_cache_offset, xnu::Mach::VmAddress* address,
                              Offset* slide);

    void cacheGetSymtabStrtab(struct symtab_command* symtab_command, xnu::Mach::VmAddress* symtab,
                              xnu::Mach::VmAddress* strtab, Offset* slide);

    xnu::Mach::VmAddress getImageLoadedAt(char* image_name, char** image_path);
    xnu::Mach::VmAddress getImageSlide(xnu::Mach::VmAddress address);

    Size getAdjustedLinkeditSize(xnu::Mach::VmAddress address);
    Size getAdjustedStrtabSize(struct symtab_command* symtab_command, xnu::Mach::VmAddress linkedit,
                               Offset linkedit_fileoff);

    void rebuildSymtabStrtab(struct symtab_command* symtab_command, xnu::Mach::VmAddress symtab_,
                             xnu::Mach::VmAddress strtab_, xnu::Mach::VmAddress linkedit,
                             Offset linkedit_fileoff);

    void fixupObjectiveC(MachO* macho);
    void fixupDyldRebaseBindOpcodes(MachO* macho, Segment* linkedit);

    Size getImageSize(xnu::Mach::VmAddress address);

    MachO* cacheDumpImage(char* image);
    MachO* cacheDumpImageToFile(char* image, char* path);

    Library* injectLibrary(const char* path);

private:
    char* main_image_path;

    xnu::Kernel* kernel;

    xnu::Task* task;

    std::vector<Library*> libraries;

    xnu::Mach::VmAddress main_image_load_base;

    xnu::Mach::VmAddress dyld;
    xnu::Mach::VmAddress dyld_shared_cache;

    Offset slide;

    xnu::Mach::VmAddress all_image_info_addr;

    Size all_image_info_size;

    struct dyld_all_image_infos* all_image_infos;
    struct dyld_image_info* main_image_info;
};

}; // namespace dyld
