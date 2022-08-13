#include "Dyld.hpp"
#include "Kernel.hpp"
#include "MachO.hpp"
#include "Task.hpp"
#include "Offset.hpp"

#include <stdio.h>
#include <stdlib.h>

#include <mach/mach.h>

static int EndsWith(const char *str, const char *suffix)
{
	if (!str || !suffix)
		return 0;
	
	size_t lenstr = strlen(str);
	size_t lensuffix = strlen(suffix);
	
	if (lensuffix >  lenstr)
		return 0;
	return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

Dyld::Dyld(Kernel *kernel, Task *task)
{
	struct mach_header_64 hdr;

	struct dyld_all_image_infos all_images;

	this->kernel = kernel;
	this->task = task;

	this->all_image_info_addr = kernel->read64(task->getTask() + Offset::task_all_image_info_addr);
	this->all_image_info_size = kernel->read64(task->getTask() + Offset::task_all_image_info_size);

	if(!all_image_info_addr || !all_image_info_size)
	{
		kern_return_t kr;

		struct task_dyld_info dyld_info;

		mach_msg_type_number_t count = TASK_DYLD_INFO_COUNT;

		if((kr = task_info(task->getTaskPort(), TASK_DYLD_INFO, (task_info_t) &dyld_info, &count)) == KERN_SUCCESS)
		{
			this->all_image_info_addr = dyld_info.all_image_info_addr;
			this->all_image_info_size = dyld_info.all_image_info_size;
		} else
		{
			MAC_RK_LOG("MacRK::could not find all_image_info for task! %d\n", kr);
		}
	}

	task->read(all_image_info_addr, &all_images, sizeof(struct dyld_all_image_infos));

	this->all_image_infos = reinterpret_cast<struct dyld_all_image_infos*> (malloc(sizeof(struct dyld_all_image_infos)));

	memcpy(this->all_image_infos, &all_images, sizeof(struct dyld_all_image_infos));

	for(uint32_t i = 0; i < all_image_infos->infoArrayCount; i++)
	{
		struct dyld_image_info image_info;

		mach_vm_address_t image_info_addr;

		mach_vm_address_t image_load_addr;
		mach_vm_address_t image_file_path;

		char *image_file;

		image_info_addr = (mach_vm_address_t) (all_images.infoArray + i);

		task->read(image_info_addr, &image_info, sizeof(image_info));

		image_load_addr = (mach_vm_address_t) image_info.imageLoadAddress;
		image_file_path = (mach_vm_address_t) image_info.imageFilePath;

		image_file = task->readString(image_file_path);

		if(EndsWith(image_file, task->getName()))
		{
			task->read(image_load_addr, &hdr, sizeof(hdr));

			if(hdr.magic == MH_MAGIC_64)
			{
				this->main_image_info = (struct dyld_image_info*) malloc(sizeof(struct dyld_image_info));

				memcpy(this->main_image_info, &image_info, sizeof(image_info));

				this->dyld_shared_cache = this->all_image_infos->sharedCacheBaseAddress;

				printf("%s main image loaded at 0x%llx\n", image_file, image_load_addr);

				this->main_image_load_base = image_load_addr;
			}
		}

		free(image_file);
	}

	this->dyld = this->getImageLoadedAt("dyld", NULL);
}

Dyld::~Dyld()
{
	
}

mach_vm_address_t Dyld::getImageLoadedAt(char *image_name, char **image_path)
{
	struct mach_header_64 hdr;

	struct dyld_all_image_infos all_images;

	task->read(this->all_image_info_addr, &all_images, sizeof(all_images));

	if(!this->all_image_infos)
	{
		this->all_image_infos = reinterpret_cast<struct dyld_all_image_infos*> (malloc(sizeof(struct dyld_all_image_infos)));
	}

	memcpy(this->all_image_infos, &all_images, sizeof(struct dyld_all_image_infos));

	for(uint32_t i = 0; i < all_image_infos->infoArrayCount; i++)
	{
		struct dyld_image_info image_info;

		mach_vm_address_t image_info_addr;

		mach_vm_address_t image_load_addr;
		mach_vm_address_t image_file_path;

		char *image_file;

		image_info_addr = (mach_vm_address_t) (all_image_infos->infoArray + i);

		task->read(image_info_addr, &image_info, sizeof(image_info));

		image_load_addr = (mach_vm_address_t) image_info.imageLoadAddress;
		image_file_path = (mach_vm_address_t) image_info.imageFilePath;

		image_file = task->readString(image_file_path);

		if(EndsWith(image_file, image_name))
		{
			task->read(image_load_addr, &hdr, sizeof(hdr));

			if(hdr.magic == MH_MAGIC_64)
			{
				if(image_path)
					*image_path = image_file;

				return image_load_addr;
			}
		}

		free(image_file);
	}

	if(image_path)
		*image_path = NULL;

	return 0;
}

struct dyld_cache_header* Dyld::cacheGetHeader()
{
	struct dyld_cache_header *cache_header;

	mach_vm_address_t shared_cache_rx_base;

	shared_cache_rx_base = this->dyld_shared_cache;

	cache_header = reinterpret_cast<struct dyld_cache_header*> (malloc(sizeof(struct dyld_cache_header)));

	task->read(shared_cache_rx_base, cache_header, sizeof(struct dyld_cache_header));

	return cache_header;
}

struct shared_file_mapping_np* Dyld::cacheGetMapping(struct dyld_cache_header *cache_header, vm_prot_t prot)
{
	mach_vm_address_t shared_cache_rx_base;

	shared_cache_rx_base = this->dyld_shared_cache;

	for(uint32_t i = 0; i < cache_header->mappingCount; ++i)
	{
		struct shared_file_mapping_np *mapping = (struct shared_file_mapping_np*) malloc(sizeof(struct shared_file_mapping_np));

		task->read(shared_cache_rx_base + cache_header->mappingOffset + sizeof(struct shared_file_mapping_np) * i, mapping, sizeof(struct shared_file_mapping_np));

		if(mapping->init_prot == prot)
			return mapping;

		free(mapping);
	}

	return NULL;
}

void Dyld::cacheGetLinkeditAddress(off_t dyld_cache_offset, mach_vm_address_t *address, off_t *aslr)
{
	struct dyld_cache_header *cache_header;

	struct shared_file_mapping_np *rx;
	struct shared_file_mapping_np *ro;
	struct shared_file_mapping_np *rw;

	mach_vm_address_t shared_cache_rx_base;

	shared_cache_rx_base = this->dyld_shared_cache;

	cache_header = this->cacheGetHeader();

	rx = this->cacheGetMapping(cache_header, VM_PROT_READ | VM_PROT_EXECUTE);

	ro = this->cacheGetMapping(cache_header, VM_PROT_READ);

	rw = this->cacheGetMapping(cache_header, VM_PROT_READ | VM_PROT_WRITE);

	free(cache_header);

	size_t rx_size = (size_t) rx->size;
	size_t rw_size = (size_t) rw->size;

	mach_vm_address_t rx_addr = (uint64_t) rx->address;
	mach_vm_address_t ro_addr = (uint64_t) ro->address;

	off_t aslr_slide = (off_t)(shared_cache_rx_base - rx->address);

	char *shared_cache_ro = (char*)((mach_vm_address_t) ro->address + aslr_slide);

	off_t offset_from_ro = (off_t) (dyld_cache_offset - (mach_vm_address_t) rx->size - (mach_vm_address_t) rw->size);

	if(address)
		*address = ((mach_vm_address_t) shared_cache_ro + offset_from_ro);

	if(slide)
		*aslr = aslr_slide;

	free(rx);
	free(ro);
	free(rw);
}

void Dyld::cacheGetSymtabStrtab(struct symtab_command *symtab_command, off_t dyld_cache_offset, mach_vm_address_t *symtab, mach_vm_address_t *strtab, off_t *slide)
{
	 this->cacheGetLinkeditAddress(symtab_command->symoff,
									symtab,
									slide);

	 this->cacheGetLinkeditAddress(symtab_command->stroff,
									strtab,
									slide);

	this->slide = *slide;
}

MachO* Dyld::cacheDumpLibrary(char *library)
{
	return NULL;
}

MachO* Dyld::cacheDumpLibraryToFile(char *library, char *path)
{
	FILE *out;

	MachO *macho = this->cacheDumpLibrary(library);

	if(!macho)
		return NULL;

	out = fopen(path, "w");

	if(!out)
		return NULL;

	fclose(out);

	return macho;
}