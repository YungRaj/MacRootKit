#ifndef __DYLD_H_
#define __DYLD_H_

#include <mach/mach_types.h>

#include <sys/types.h>

#include <dyld_cache_format.h>

#include <mach-o.h>

class MachO;
class Segment;
class Section;

namespace xnu
{
	class Task;
	class Kernel;
};

#define MH_DYLIB_IN_CACHE 0x80000000

namespace dyld
{
	class Dyld
	{
		public:
			Dyld(xnu::Kernel *kernel, xnu::Task *task);

			~Dyld();

			char* getMainImagePath() { return main_image_path; }

			mach_vm_address_t getMainImageLoadBase() { return main_image_load_base; }
			mach_vm_address_t getAllImageInfoAddr() { return all_image_info_addr; }

			mach_vm_address_t getDyld() { return dyld; }
			mach_vm_address_t getDyldSharedCache() { return dyld_shared_cache; }

			struct dyld_image_info* getMainImageInfo() { return main_image_info; }

			off_t getSlide() { return slide; }

			void getImageInfos();

			struct dyld_cache_header* cacheGetHeader();

			struct dyld_cache_mapping_info* cacheGetMappings(struct dyld_cache_header *cache_header);
			struct dyld_cache_mapping_info* cacheGetMapping(struct dyld_cache_header *cache_header, vm_prot_t prot);

			void cacheOffsetToAddress(uint64_t dyld_cache_offset, mach_vm_address_t *address, off_t *slide);

			void cacheGetSymtabStrtab(struct symtab_command *symtab_command, mach_vm_address_t *symtab, mach_vm_address_t *strtab, off_t *slide);

			mach_vm_address_t getImageLoadedAt(char *image_name, char **image_path);
			mach_vm_address_t getImageSlide(mach_vm_address_t address);

			size_t getAdjustedLinkeditSize(mach_vm_address_t address);
			size_t getAdjustedStrtabSize(struct symtab_command *symtab_command, mach_vm_address_t linkedit, off_t linkedit_fileoff);

			void rebuildSymtabStrtab(struct symtab_command *symtab_command, mach_vm_address_t symtab_, mach_vm_address_t strtab_, mach_vm_address_t linkedit, off_t linkedit_fileoff);

			void fixupObjectiveC(MachO *macho);
			void fixupDyldRebaseBindOpcodes(MachO *macho, Segment *linkedit);

			size_t getImageSize(mach_vm_address_t address);

			MachO* cacheDumpImage(char *image);
			MachO* cacheDumpImageToFile(char *image, char *path);

		private:
			char *main_image_path;

			xnu::Kernel *kernel;

			xnu::Task *task;

			mach_vm_address_t main_image_load_base;

			mach_vm_address_t dyld;
			mach_vm_address_t dyld_shared_cache;

			off_t slide;

			mach_vm_address_t all_image_info_addr;
			
			size_t all_image_info_size;

			struct dyld_all_image_infos *all_image_infos;
			struct dyld_image_info *main_image_info;
	};

};

#endif