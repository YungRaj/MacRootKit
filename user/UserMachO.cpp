#include <stdio.h>
#include <stdlib.h>

#include "UserMachO.hpp"
#include "ObjC.hpp"

#include "Task.hpp"
#include "Dyld.hpp"

using namespace mrk;

UserMachO::UserMachO(const char *path)
{
	this->objc = NULL;
	this->file_path = strdup(path);

	this->initWithFilePath(path);
}

void UserMachO::initWithTask(Task *task)
{
	this->task = task;
	this->dyld = task->getDyld(); 
	this->dyld_base = this->dyld->getDyld();
	this->dyld_shared_cache = this->dyld->getDyldSharedCache();
	this->file_path = dyld->getMainImagePath();
	this->symbolTable = new SymbolTable();
}

void UserMachO::initWithFilePath(const char *path)
{
	FILE *file;

	size_t size;

	file = fopen(path, "r");

	if(!file) return;

	fseek(file, 0, SEEK_END);
	
	size = ftell(file);

	fseek(file, 0, SEEK_SET);

	this->buffer = (char*) malloc(size);

	fseek(file, 0, SEEK_SET);
	fread(this->buffer, 1, size, file);

	this->header = reinterpret_cast<struct mach_header_64*>(this->buffer);
	this->base = reinterpret_cast<mach_vm_address_t>(this->buffer);

	fclose(file);

	this->symbolTable = new SymbolTable();

	this->parseMachO();
}

void UserMachO::initWithBuffer(char *buf)
{
	buffer = buf;

	header = (struct mach_header_64*) buffer;

	this->symbolTable = new SymbolTable();

	this->parseMachO();
}

void UserMachO::initWithBuffer(char *buf, off_t slide)
{
	buffer = buf;

	header = (struct mach_header_64*) buffer;

	this->symbolTable = new SymbolTable();
	this->aslr_slide = slide;

	this->parseMachO();
}

void UserMachO::initWithBuffer(mach_vm_address_t base_, char *buf, off_t slide)
{
	buffer = buf;
	base = base_;

	header = (struct mach_header_64*) buffer;

	this->setIsDyldCache(false);

	this->symbolTable = new SymbolTable();
	this->aslr_slide = slide;

	this->parseMachO();
}

void UserMachO::initWithBuffer(mach_vm_address_t base_, char *buf, off_t slide, bool is_dyld_cache)
{
	buffer = buf;
	base = base_;

	header = (struct mach_header_64*) buffer;

	this->setIsDyldCache(is_dyld_cache);

	this->symbolTable = new SymbolTable();
	this->aslr_slide = slide;

	this->parseMachO();
}

void UserMachO::initWithBuffer(UserMachO *libobjc, mach_vm_address_t base_, char *buf, off_t slide)
{
	buffer = buf;
	base = base_;

	header = (struct mach_header_64*) buffer;

	this->setIsDyldCache(true);

	this->libobjc = libobjc;
	this->symbolTable = new SymbolTable();
	this->aslr_slide = slide;

	this->parseMachO();
}

void UserMachO::initWithBuffer(char *buffer, uint64_t size)
{

}

MachO* UserMachO::libraryLoadedAt(mach_port_t task_port, char *library)
{
	return NULL;
}


uint64_t UserMachO::untagPacPointer(mach_vm_address_t base, enum dyld_fixup_t fixupKind, uint64_t ptr, bool *bind, bool *auth, uint16_t *pac, size_t *skip)
{
	pacptr_t pp;

	pp.ptr = ptr;

	if(fixupKind == DYLD_CHAINED_PTR_64_KERNEL_CACHE)
	{
		if(bind) *bind = false;
		if(auth) *auth = !!(pp.cache.auth && pp.cache.tag);
		if(pac)  *pac  = pp.cache.div;
		if(skip) *skip = pp.cache.next * sizeof(uint32_t);

		return base + pp.cache.target;
	}

	if(fixupKind == DYLD_CHAINED_PTR_ARM64E || fixupKind == DYLD_CHAINED_PTR_ARM64E_KERNEL)
	{
		if(pp.pac.bind)
		{
			if(bind) *bind = true;
		} else if(bind)
		{
			if(bind) *bind = false;
		}

		if(auth) *auth = false;
		if(pac)  *pac  = 0;
		if(skip) *skip = pp.pac.next * sizeof(uint32_t);
		if(pp.pac.auth)
		{
			if(auth) *auth = !!pp.pac.tag;
			if(pac)  *pac  = pp.pac.div;
		}
		if(pp.pac.bind) return pp.pac.off & 0xffff;
		if(pp.pac.auth) return base + pp.pac.off;

		return (uint64_t)pp.raw.lo;
	}

	if(bind) *bind = false;
	if(auth) *auth = false;
	if(pac)  *pac  = 0;
	if(skip) *skip = 0;

	return pp.ptr;
}

bool UserMachO::isPointerInPacFixupChain(mach_vm_address_t ptr)
{
	struct mach_header_64 *header;

	uint8_t *q;

	uint64_t ncmds;

	uint64_t cmd_offset;

	bool bind;

	header = this->getMachHeader();

	ncmds = header->ncmds;

	cmd_offset = sizeof(struct mach_header_64);

	q = (uint8_t*) header + cmd_offset;

	for(int n = 0; n < ncmds; n++)
	{
		struct load_command *load_cmd = (struct load_command*) q;

		uint32_t cmdtype = load_cmd->cmd;
		uint32_t cmdsize = load_cmd->cmdsize;

		if(cmdtype == LC_SEGMENT_64)
		{
			struct segment_command_64 *segcmd = (struct segment_command_64*) load_cmd;

			if(strcmp("__TEXT", segcmd->segname) == 0)
			{
				uint64_t sect_offset = cmd_offset + sizeof(struct segment_command_64);

				for(int j = 0; j < segcmd->nsects; j++)
				{
					struct section_64 *section = (struct section_64*) (q + sect_offset);

					if(strcmp("__thread_starts", section->sectname) == 0)
					{
						uint32_t *start = (uint32_t*) ((uintptr_t) ((uint8_t*) header + section->offset));
						uint32_t *end = (uint32_t*) ((uintptr_t) start + section->size);

						if(end > start)
						{
							++start;

							for(; start < end; ++start)
							{
								if(*start == 0xffffffff)
									break;

								uint64_t *mem = (uint64_t*) ((uint8_t*) header + *start);

								size_t skip = 0;

								do
								{
									uint64_t ptr_tag = *mem;
									uint64_t ptr_tag_ptr = ((uint64_t) mem - (uint64_t) header) + this->getBase();

									ptr_tag = this->untagPacPointer(this->getBase(), DYLD_CHAINED_PTR_ARM64E, ptr_tag, &bind, NULL, NULL, &skip);
								
									if(ptr_tag_ptr == ptr)
									{
										return true;
									}

									mem = (uint64_t*) ((uint64_t) mem + skip);

								} while(skip > 0);

								return false;
							}
						}

						sect_offset += sizeof(struct section_64);
					}
				}
			}
		}

		if(cmdtype == LC_DYLD_CHAINED_FIXUPS)
		{
			struct linkedit_data_command *data = (struct linkedit_data_command*) q;

			struct dyld_chained_fixups_header *fixup = (struct dyld_chained_fixups_header*) (uint8_t*) header + data->dataoff;
			struct dyld_chained_starts_in_image *segs = (struct dyld_chained_starts_in_image*)((uintptr_t)fixup + fixup->starts_offset);

			for(uint32_t i = 0; i < segs->seg_count; ++i)
			{
				if(segs->seg_info_offset[i] == 0)
				{
					continue;
				}

				struct dyld_chained_starts_in_segment *starts = (struct dyld_chained_starts_in_segment*)((uintptr_t)segs + segs->seg_info_offset[i]);
				
				uint64_t off = (uintptr_t)ptr - (uintptr_t)this->buffer;

				if(starts->pointer_format == DYLD_CHAINED_PTR_ARM64E_KERNEL)
				{
					off = this->offsetToAddress(off) - this->base;
				}

				if(off < starts->segment_offset || (off - starts->segment_offset) / starts->page_size >= starts->page_count)
				{
					continue;
				}

				uint16_t j = (off - starts->segment_offset) / starts->page_size;
				uint16_t idx = starts->page_start[j];

				if(idx == 0xffff)
				{
					continue;
				}

				size_t where = (size_t)starts->segment_offset + (size_t)j * (size_t)starts->page_size + (size_t)idx;
				size_t skip = 0;

				uint64_t *mem = starts->pointer_format == DYLD_CHAINED_PTR_ARM64E_KERNEL ? reinterpret_cast<uint64_t*>(this->addressToPointer(this->base + where))
																						 : reinterpret_cast<uint64_t*>((reinterpret_cast<uintptr_t>(this->buffer) + where));
				
				do
				{
					uint64_t ptr_tag = *mem;
					uint64_t ptr_tag_ptr = ((uint64_t) mem - (uint64_t) this->buffer) + this->base;

					ptr_tag = this->untagPacPointer(this->base, DYLD_CHAINED_PTR_ARM64E, ptr_tag, &bind, NULL, NULL, &skip);

					if(ptr_tag_ptr == ptr)
					{
						return true;
					}

					mem = (uint64_t*)((uintptr_t)mem + skip);

				} while(skip > 0);
			}

			return false;
		}

		cmd_offset += cmdsize;
	}

	return false;
}

mach_vm_address_t UserMachO::getBufferAddress(mach_vm_address_t address)
{
	mach_vm_address_t header = reinterpret_cast<mach_vm_address_t>(this->getMachHeader());

	Segment *segment = this->segmentForAddress(address);

	Section *section = this->sectionForAddress(address);

	if(segment && !section)
	{
		return header + segment->getFileOffset() + (address - segment->getAddress());
	}

	if(!segment && !section)
	{
		address -= this->getAslrSlide();

		segment = this->segmentForAddress(address);
		section = this->sectionForAddress(address);
	}

	return segment && section ? header + section->getOffset() + (address - section->getAddress()) : 0;
}

void UserMachO::parseSymbolTable(struct nlist_64 *symtab, uint32_t nsyms, char *strtab, size_t strsize)
{
	for(int i = 0; i < nsyms; i++)
	{
		Symbol *symbol;

		struct nlist_64 *nl = &symtab[i];

		char *name;

		mach_vm_address_t address;

		name = &strtab[nl->n_strx];

		address = nl->n_value;

		printf("%s\n", name);

		symbol = new Symbol(this, nl->n_type & N_TYPE, name, address, this->addressToOffset(address), this->segmentForAddress(address), this->sectionForAddress(address));

		this->symbolTable->addSymbol(symbol);
	}
}

uint64_t UserMachO::readUleb128(uint8_t *start, uint8_t *end, uint32_t *idx)
{
	uint64_t result = 0;

	uint8_t *p = start;

	int bit = 0;

	do
	{
		if(p == end)
		{
			break;
		}

		uint64_t slice = *p & 0x7F;

		if(bit > 63)
			break;
		else
		{
			result |= (slice << bit);

			bit += 0x7;
		}

		*idx = *idx + 1;

	} while(*p++ & 0x80);

	*idx = *idx - 1;

	return result;
}

int64_t UserMachO::readSleb128(uint8_t *start, uint8_t *end, uint32_t *idx)
{
	int64_t result = 0;

	uint8_t *p = start;

	int bit = 0;

	uint8_t byte = 0;

	do
	{
		if(p == end)
			break;

		byte = *p++;

		*idx = *idx + 1;

		result |= (((int64_t) (byte & 0x7F)) << bit);

		bit += 0x7;

	} while(byte & 0x90);

	*idx = *idx - 1;

	if(((byte & 0x40) != 0) && (bit < 64))
		result |= (~0ULL) << bit;

	return result;
}

void UserMachO::parseLinkedit()
{

}

bool UserMachO::parseLoadCommands()
{
	struct mach_header_64 *mh = this->getMachHeader();

	size_t file_size;

	this->size = this->getSize();

	file_size = this->size;

	uint8_t *q = (uint8_t*) mh + sizeof(struct mach_header_64);

	uint32_t current_offset = sizeof(struct mach_header_64);

	for(uint32_t i = 0; i < mh->ncmds; i++)
	{
		struct load_command *load_cmd = (struct load_command*) this->getOffset(current_offset);

		uint32_t cmdtype = load_cmd->cmd;
		uint32_t cmdsize = load_cmd->cmdsize;

		if(cmdsize > mh->sizeofcmds - ((uintptr_t) load_cmd - (uintptr_t)(mh + 1)))
			return false;

		switch(cmdtype)
		{
			case LC_LOAD_DYLIB:
			{
				;
				struct dylib_command *dylib_command = (struct dylib_command*) load_cmd;
				struct dylib dylib = dylib_command->dylib;

				off_t dylib_name_offset = current_offset  + reinterpret_cast<off_t>(dylib.name);
				size_t dylib_name_len = cmdsize - sizeof(dylib_command);

				char *name = (char*) this->getOffset(dylib_name_offset);

				// printf("LC_LOAD_DYLIB - %s\n",name);
				// printf("\tVers - %u Timestamp - %u\n", dylib.current_version,dylib.timestamp);

				break;
			}

			case LC_SEGMENT_64:
			{
				;
				struct segment_command_64 *segment_command = (struct segment_command_64*) load_cmd;

				uint32_t nsects = segment_command->nsects;
				uint32_t sect_offset = current_offset + sizeof(struct segment_command_64);

				if(segment_command->fileoff > this->size || segment_command->filesize > this->size - segment_command->fileoff)
					return false;

				printf("LC_SEGMENT_64 at 0x%llx - %s 0x%08llx to 0x%08llx \n", segment_command->fileoff,
		                                  segment_command->segname,
		                                  segment_command->vmaddr,
		                                  segment_command->vmaddr + segment_command->vmsize);
				int j = 0;

				if(nsects * sizeof(struct section_64) + sizeof(struct segment_command_64) > cmdsize)
					return false;

				Segment *segment = new Segment(segment_command);

				for(j = 0; j < nsects; j++)
				{
					struct section_64 *section = (struct section_64*) this->getOffset(sect_offset);

					printf("\tSection %d: 0x%08llx to 0x%08llx - %s\n", j,
					                            section->addr,
					                            section->addr + section->size,
					                            section->sectname);

					if(section->offset > this->size || section->size > this->size - section->offset)
					{
						return false;
					}

					sect_offset += sizeof(struct section_64);
				}

				this->segments.add(segment);

				break;
			}

			case LC_SYMTAB:
			{
				;
				struct symtab_command *symtab_command = (struct symtab_command*) load_cmd;

				if(symtab_command->stroff > this->size || symtab_command->symoff > this->size || symtab_command->nsyms > (this->size - symtab_command->symoff) / sizeof(struct nlist_64))
					return false;

				// printf("LC_SYMTAB\n");
				// printf("\tSymbol Table is at offset 0x%x (%u) with %u entries \n",symtab_command->symoff,symtab_command->symoff,symtab_command->nsyms);
				// printf("\tString Table is at offset 0x%x (%u) with size of %u bytes\n",symtab_command->stroff,symtab_command->stroff,symtab_command->strsize);

				struct nlist_64 *symtab = (struct nlist_64*) this->getOffset(symtab_command->symoff);
				uint32_t nsyms = symtab_command->nsyms;

				char *strtab = (char*) this->getOffset(symtab_command->stroff);
				uint32_t strsize = symtab_command->strsize;

				if(nsyms > 0)
					this->parseSymbolTable(symtab, nsyms, strtab, strsize);

				break;
			}

			case LC_DYSYMTAB:
			{
				;
				struct dysymtab_command *dysymtab_command = (struct dysymtab_command*) load_cmd;

				if(dysymtab_command->extreloff > this->size || dysymtab_command->nextrel > (this->size - dysymtab_command->extreloff) / sizeof(struct relocation_info))
					return false;

				// printf("LC_DYSYMTAB\n");
				// printf("\t%u local symbols at index %u\n",dysymtab_command->ilocalsym,dysymtab_command->nlocalsym);
				// printf("\t%u external symbols at index %u\n",dysymtab_command->nextdefsym,dysymtab_command->iextdefsym);
				// printf("\t%u undefined symbols at index %u\n",dysymtab_command->nundefsym,dysymtab_command->iundefsym);
				// printf("\t%u Indirect symbols at offset 0x%x\n",dysymtab_command->nindirectsyms,dysymtab_command->indirectsymoff);

				break;
			}

			case LC_UUID:
			{
				;
				struct uuid_command *uuid_command = (struct uuid_command*) load_cmd;

				if(uuid_command->cmdsize != sizeof(struct uuid_command))
					return false;

				/*
				printf("LC_UUID\n");
				printf("\tuuid = ");
				*/

				// for(int j = 0; j < sizeof(uuid_command->uuid); j++)
					// printf("%x", uuid_command->uuid[j]);

				// printf("\n");

				break;
			}

			case LC_FUNCTION_STARTS:
			{
				;
				struct linkedit_data_command *linkedit = (struct linkedit_data_command*) load_cmd;

				uint32_t dataoff = linkedit->dataoff;
				uint32_t datasize = linkedit->datasize;

				// printf("LC_FUNCTION_STARTS\n");
				// printf("\tOffset = 0x%x Size = 0x%x\n", dataoff, datasize);

				break;
			}

			case LC_MAIN:
			{
				;
				struct entry_point_command *ep = (struct entry_point_command*) load_cmd;

				// printf("LC_MAIN\n");
				// printf("\tEntry Point = 0x%llx\n", macho->base + entry_point->entryoff);

				this->entry_point = this->base + ep->entryoff;

				break;
			}

			case LC_UNIXTHREAD:
			{
				;
				struct unixthread_command *thread_command = (struct unixthread_command*) load_cmd;

				// if(thread_command->flavor != ARM_THREAD_STATE64)
				//	return false;

				break;
			}

			case LC_DYLD_INFO:
			{
				;
				struct dyld_info_command *dyld_info_command = (struct dyld_info_command*) load_cmd;

				uint32_t rebase_off = dyld_info_command->rebase_off;
				uint32_t rebase_size = dyld_info_command->rebase_size;

				uint32_t bind_off = dyld_info_command->bind_off;
				uint32_t bind_size = dyld_info_command->bind_size;

				uint32_t weak_bind_off = dyld_info_command->weak_bind_off;
				uint32_t weak_bind_size = dyld_info_command->weak_bind_size;

				uint32_t lazy_bind_off = dyld_info_command->lazy_bind_off;
				uint32_t lazy_bind_size = dyld_info_command->lazy_bind_size;

				uint32_t export_off = dyld_info_command->export_off;
				uint32_t export_size = dyld_info_command->export_size;

				/* printf("LC_DYLD_INFO\n");

				if(rebase_off)
					printf("\tRebase info: %u bytes at offset 0x%x\n", rebase_size, rebase_off);
				else
					printf("\tNo rebase info\n");

				if(bind_off)
					printf("\tBind info: %u bytes at offset 0x%x\n", bind_size, bind_off);
				else
					printf("\tNo bind info\n");

				if(weak_bind_off)
					printf("\tWeak info: %u bytes at offset 0x%x\n", weak_bind_size, weak_bind_off);
				else
					printf("\tNo weak info\n");

				if(lazy_bind_off)
					printf("\tLazy info: %u bytes at offset 0x%x\n", lazy_bind_size, lazy_bind_off);
				else
					printf("\tNo lazy info\n");

				if(export_off)
					printf("\tExport info: %u bytes at offset 0x%x\n", export_size, export_off);
				else
					printf("\tNo export info\n");
				*/

				break;
			}

			case LC_DYLD_INFO_ONLY:
			{
				;
				struct dyld_info_command *dyld_info_command = (struct dyld_info_command*) load_cmd;

				uint32_t rebase_off = dyld_info_command->rebase_off;
				uint32_t rebase_size = dyld_info_command->rebase_size;

				uint32_t bind_off = dyld_info_command->bind_off;
				uint32_t bind_size = dyld_info_command->bind_size;

				uint32_t weak_bind_off = dyld_info_command->weak_bind_off;
				uint32_t weak_bind_size = dyld_info_command->weak_bind_size;

				uint32_t lazy_bind_off = dyld_info_command->lazy_bind_off;
				uint32_t lazy_bind_size = dyld_info_command->lazy_bind_size;

				uint32_t export_off = dyld_info_command->export_off;
				uint32_t export_size = dyld_info_command->export_size;

				/*
				printf("LC_DYLD_INFO_ONLY\n");

				if(rebase_off)
				{
					printf("\tRebase info: %u bytes at offset 0x%x\n", rebase_size, rebase_off);

					macho_parse_dyld_rebase_info(macho, rebase_off, rebase_size);
				}
				else
					printf("\tNo rebase info\n");

				if(bind_off)
				{
					printf("\tBind info: %u bytes at offset 0x%x\n", bind_size, bind_off);

					macho_parse_dyld_bind_info(macho, bind_off, bind_size);
				}
				else
					printf("\tNo bind info\n");

				if(weak_bind_off)
				{
					printf("\tWeak info: %u bytes at offset 0x%x\n", weak_bind_size, weak_bind_off);

					macho_parse_dyld_bind_info(macho, weak_bind_off, weak_bind_size);
				}
				else
					printf("\tNo weak info\n");

				if(lazy_bind_off)
				{
					printf("\tLazy info: %u bytes at offset 0x%x\n", lazy_bind_size, lazy_bind_off);

					macho_parse_dyld_bind_info(macho, lazy_bind_off, lazy_bind_size);
				}
				else
					printf("\tNo lazy info\n");

				if(export_off)
					printf("\tExport info: %u bytes at offset 0x%x\n", export_size, export_off);
				else
					printf("\tNo export info\n");
				*/

				break;
			}

			case LC_CODE_SIGNATURE:
			{
				struct linkedit_data_command *linkedit = (struct linkedit_data_command*) load_cmd;

				uint32_t dataoff = linkedit->dataoff;
				uint32_t datasize = linkedit->datasize;

				// printf("LC_CODE_SIGNATURE\n");

				this->parseCodeSignature(linkedit);

				break;
			}

			case LC_DATA_IN_CODE:
			{
				;
				struct linkedit_data_command *linkedit = (struct linkedit_data_command*) load_cmd;

				uint32_t dataoff = linkedit->dataoff;
				uint32_t datasize = linkedit->datasize;

				// printf("LC_DATA_IN_CODE\n");
				// printf("\tOffset = 0x%x Size = 0x%x\n", dataoff, datasize);

				break;
			}
		}

		current_offset += cmdsize;
	}

	if(this->getSection("__DATA_CONST", "__objc_classlist"))
	{
		this->parseObjC();
	}

	return true;
}

void UserMachO::parseFatHeader()
{
	struct fat_header *fat_header = (struct fat_header*) this->getMachHeader();

	struct fat_arch *arch = (struct fat_arch*) ((uint8_t*) fat_header + sizeof(struct fat_header));

	uint32_t nfat_arch = OSSwapBigToHostInt32(fat_header->nfat_arch);

	for(uint32_t i = 0; i < fat_header->nfat_arch; i++)
	{
		uint32_t cputype;
		uint32_t cpusubtype;

		uint64_t offset;

		cputype = OSSwapBigToHostInt32(arch->cputype);
		cpusubtype = OSSwapBigToHostInt32(arch->cpusubtype);

		offset = OSSwapBigToHostInt32(arch->offset);

#ifdef __x86_64__
	#define NATIVE_CPU_TYPE 	CPU_TYPE_X86_64
#endif

#ifdef __arm64__
	#define NATIVE_CPU_TYPE 	CPU_TYPE_ARM64
#endif

		if(cputype == NATIVE_CPU_TYPE)
		{
			struct mach_header_64 *mh = (struct mach_header_64*) ((uint8_t*) fat_header + offset);

			this->buffer = reinterpret_cast<char*>(mh);
			this->header = mh;

			this->parseHeader();
		} 

		arch++;
	}

}

void UserMachO::parseHeader()
{
	struct mach_header_64 *mh = (struct mach_header_64*) this->getMachHeader();

	uint32_t magic = mh->magic;

	if(magic == FAT_CIGAM)
	{
		// this->parseFatHeader();
		// ignore fatHeader if we are parsing in memory MachO's that aren't dumps
	} 

	else if(magic == MH_MAGIC_64)
	{
		this->parseLoadCommands();
	}
}

void UserMachO::parseMachO()
{
	parseHeader();
}

