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

#include "KextMachO.hpp"

extern "C"
{
	#include <fcntl.h>
};

KextMachO::KextMachO(uintptr_t base)
{
	buffer = reinterpret_cast<char*>(base),
	header = reinterpret_cast<struct mach_header_64*>(this->buffer);
	base = reinterpret_cast<xnu::Mach::VmAddress>(this->buffer);
	symbolTable = new SymbolTable();
	aslr_slide = 0;

	this->parseMachO();
}

KextMachO::KextMachO(uintptr_t base, Offset slide)
{
	buffer = reinterpret_cast<char*>(base),
	header = reinterpret_cast<struct mach_header_64*>(this->buffer);
	base = reinterpret_cast<xnu::Mach::VmAddress>(this->buffer);
	symbolTable = new SymbolTable();
	aslr_slide = 0;

	this->parseMachO();
}

KextMachO::KextMachO(const char *path, Offset slide)
{
	int fd = open(path, O_RDONLY);

	Size size = lseek(fd, 0, SEEK_END);

	lseek(fd, 0, SEEK_SET);

	this->buffer = reinterpret_cast<char*>(malloc(size));

	Size bytes_read;

	bytes_read = read(fd, this->buffer, size);

	this->header = reinterpret_cast<struct mach_header_64*>(this->buffer);
	this->base = reinterpret_cast<xnu::Mach::VmAddress>(this->buffer);

	this->symbolTable = new SymbolTable();

	this->aslr_slide = slide;

	this->parseMachO();

	close(fd);
}

KextMachO::KextMachO(const char *path)
{
	int fd = open(path, O_RDONLY);

	Size size = lseek(fd, 0, SEEK_END);

	lseek(fd, 0, SEEK_SET);

	this->buffer = reinterpret_cast<char*>(malloc(size));

	Size bytes_read;

	bytes_read = read(fd, this->buffer, size);

	this->header = reinterpret_cast<struct mach_header_64*>(this->buffer);
	this->base = reinterpret_cast<xnu::Mach::VmAddress>(this->buffer);

	this->symbolTable = new SymbolTable();

	this->aslr_slide = 0;

	this->parseMachO();

	close(fd);
}

KextMachO::~KextMachO()
{
}

void KextMachO::parseLinkedit()
{
	MachO::parseLinkedit();
}

bool KextMachO::parseLoadCommands()
{
	struct mach_header_64 *mh = this->getMachHeader();

	Size file_size;

	this->size = this->getSize();

	file_size = this->size;

	UInt8 *q = reinterpret_cast<UInt8*>(mh) + sizeof(struct mach_header_64);

	UInt32 current_offset = sizeof(struct mach_header_64);

	for(UInt32 i = 0; i < mh->ncmds; i++)
	{
		struct load_command *load_command = reinterpret_cast<struct load_command*>(this->getOffset(current_offset));

		UInt32 cmdtype = load_command->cmd;
		UInt32 cmdsize = load_command->cmdsize;

		if(cmdsize > mh->sizeofcmds - ((uintptr_t) load_command - (uintptr_t)(mh + 1)))
			return false;

		switch(cmdtype)
		{
			case LC_SEGMENT_64:
			{
				;
				struct segment_command_64 *segment_command = reinterpret_cast<struct segment_command_64*>(load_command);

				UInt32 nsects = segment_command->nsects;
				UInt32 sect_offset = current_offset + sizeof(struct segment_command_64);

				if(segment_command->fileoff > this->size || segment_command->filesize > this->size - segment_command->fileoff)
					return false;

				char buffer1[128];
				char buffer2[128];

				snprintf(buffer1, 128, "0x%08llx", segment_command->vmaddr);
				snprintf(buffer2, 128, "0x%08llx", segment_command->vmaddr + segment_command->vmsize);

				MAC_RK_LOG("MacRK::LC_SEGMENT_64 at 0x%llx - %s %s to %s \n", segment_command->fileoff,
																						segment_command->segname,
																						buffer1,
																						buffer2); 

				int j = 0;

				if(nsects * sizeof(struct section_64) + sizeof(struct segment_command_64) > cmdsize)
					return false;

				Segment *segment = new Segment(segment_command);

				for(j = 0; j < nsects; j++)
				{
					struct section_64 *section = reinterpret_cast<struct section_64*>(this->getOffset(sect_offset));

					char buffer1[128];
					char buffer2[128];

					snprintf(buffer1, 128, "0x%08llx", section->addr);
					snprintf(buffer2, 128, "0x%08llx", section->addr + section->size);

					MAC_RK_LOG("MacRK::\tSection %d: %s to %s - %s\n", j,
					                            buffer1,
					                            buffer2,
					                            section->sectname); 

					if(section->offset > this->size || section->size > this->size - section->offset)
					{
						return false;
					}

					sect_offset += sizeof(struct section_64);
				}

				this->segments.push_back(segment);

				break;
			}

			case LC_SYMTAB:
			{
				;
				struct symtab_command *symtab_command = reinterpret_cast<struct symtab_command*>(load_command);

				struct nlist_64 *symtab;
				UInt32 nsyms;

				char *strtab;
				UInt32 strsize;

				if(symtab_command->stroff > this->size || symtab_command->symoff > this->size || symtab_command->nsyms > (this->size - symtab_command->symoff) / sizeof(struct nlist_64))
					return false;

				MAC_RK_LOG("MacRK::LC_SYMTAB\n");
				MAC_RK_LOG("MacRK::\tSymbol Table is at offset 0x%x (%u) with %u entries \n",symtab_command->symoff,symtab_command->symoff,symtab_command->nsyms);
				MAC_RK_LOG("MacRK::\tString Table is at offset 0x%x (%u) with size of %u bytes\n",symtab_command->stroff,symtab_command->stroff,symtab_command->strsize);

				symtab = reinterpret_cast<struct nlist_64*>(this->getBase() + symtab_command->symoff);
				nsyms = symtab_command->nsyms;

				strtab = reinterpret_cast<char*>(this->getBase() + symtab_command->stroff);
				strsize = symtab_command->strsize;

				char buffer1[128];
				char buffer2[128];

				snprintf(buffer1, 128, "0x%llx", (UInt64) symtab);
				snprintf(buffer2, 128, "0x%llx", (UInt64) strtab);

				MAC_RK_LOG("MacRK::\tSymbol Table address = %s\n", buffer1);
				MAC_RK_LOG("MacRK::\tString Table address = %s\n", buffer2);

				if(nsyms > 0)
					this->parseSymbolTable(symtab, nsyms, strtab, strsize);

				break;
			}

			case LC_DYSYMTAB:
			{
				;
				struct dysymtab_command *dysymtab_command = reinterpret_cast<struct dysymtab_command*>(load_command);

				if(dysymtab_command->extreloff > this->size || dysymtab_command->nextrel > (this->size - dysymtab_command->extreloff) / sizeof(struct relocation_info))
					return false;

				MAC_RK_LOG("MacRK::LC_DYSYMTAB\n");
				MAC_RK_LOG("MacRK::\t%u local symbols at index %u\n",dysymtab_command->ilocalsym,dysymtab_command->nlocalsym);
				MAC_RK_LOG("MacRK::\t%u external symbols at index %u\n",dysymtab_command->nextdefsym,dysymtab_command->iextdefsym);
				MAC_RK_LOG("MacRK::\t%u undefined symbols at index %u\n",dysymtab_command->nundefsym,dysymtab_command->iundefsym);
				MAC_RK_LOG("MacRK::\t%u Indirect symbols at offset 0x%x\n",dysymtab_command->nindirectsyms,dysymtab_command->indirectsymoff);

				break;
			}

			case LC_UUID:
			{
				;
				struct uuid_command *uuid_command = reinterpret_cast<struct uuid_command*>(load_command);

				if(uuid_command->cmdsize != sizeof(struct uuid_command))
					return false;

				
				MAC_RK_LOG("MacRK::LC_UUID\n");
				MAC_RK_LOG("MacRK::\tuuid = ");
				

				for(int j = 0; j < sizeof(uuid_command->uuid); j++)
					MAC_RK_LOG("%x", uuid_command->uuid[j]);

				MAC_RK_LOG("\n");

				break;
			}

			case LC_FUNCTION_STARTS:
			{
				;
				struct linkedit_data_command *linkedit = reinterpret_cast<struct linkedit_data_command*>(load_command);

				UInt32 dataoff = linkedit->dataoff;
				UInt32 datasize = linkedit->datasize;

				MAC_RK_LOG("MacRK::LC_FUNCTION_STARTS\n");
				MAC_RK_LOG("MacRK::\tOffset = 0x%x Size = 0x%x\n", dataoff, datasize);

				break;
			}

			case LC_MAIN:
			{
				;
				struct entry_point_command *ep = reinterpret_cast<struct entry_point_command*>(load_command);

				MAC_RK_LOG("MacRK::LC_MAIN\n");
				MAC_RK_LOG("MacRK::\tEntry Point = 0x%llx\n", this->base + ep->entryoff);

				this->entry_point = this->base + ep->entryoff;

				break;
			}

			case LC_UNIXTHREAD:
			{
				;
				struct unixthread_command *thread_command = reinterpret_cast<struct unixthread_command*>(load_command);

				// if(thread_command->flavor != ARM_THREAD_STATE64)
					//return false;

				break;
			}

			case LC_DATA_IN_CODE:
			{
				;
				struct linkedit_data_command *linkedit = reinterpret_cast<struct linkedit_data_command*>(load_command);

				UInt32 dataoff = linkedit->dataoff;
				UInt32 datasize = linkedit->datasize;

				MAC_RK_LOG("MacRK::LC_DATA_IN_CODE\n");
				MAC_RK_LOG("MacRK::\tOffset = 0x%x Size = 0x%x\n", dataoff, datasize);

				break;
			}
		}

		current_offset += cmdsize;
	}

	return true;
}

void KextMachO::parseMachO()
{
	MachO::parseMachO();
}