#include "MachO.hpp"
#include "Log.hpp"

MachO::MachO()
{

}

MachO::~MachO()
{
	
}

void MachO::initWithBase(mach_vm_address_t base, off_t slide)
{
	this->base = base;
	this->aslr_slide = slide;
	this->buffer = reinterpret_cast<char*>(base);
	this->header = reinterpret_cast<struct mach_header_64*>(buffer);
	this->symbolTable = new SymbolTable();
	
	this->parseMachO();
}

size_t MachO::getSize()
{
	size_t file_size = 0;

	struct mach_header_64 *mh = this->getMachHeader();

	uint8_t *q = reinterpret_cast<uint8_t*>(mh) + sizeof(struct mach_header_64);

	for(uint32_t i = 0; i < mh->ncmds; i++)
	{
		struct load_command *load_command = reinterpret_cast<struct load_command*>(q);

		uint32_t cmd = load_command->cmd;
		uint32_t cmdsize = load_command->cmdsize;

		if(cmd == LC_SEGMENT_64)
		{
			struct segment_command_64 *segment_command = reinterpret_cast<struct segment_command_64*>(load_command);

			uint64_t vmaddr = segment_command->vmaddr;
			uint64_t vmsize = segment_command->vmsize;

			uint64_t fileoff = segment_command->fileoff;
			uint64_t filesize = segment_command->filesize;

			if(vmsize > 0)
			{
				file_size = max(file_size, fileoff + filesize);
			}
		}

		q += cmdsize;
	}

	return file_size;
}

Symbol* MachO::getSymbolByName(char *symbolname)
{
	return this->symbolTable->getSymbolByName(symbolname);
}

Symbol* MachO::getSymbolByAddress(mach_vm_address_t address)
{
	return this->symbolTable->getSymbolByAddress(address);
}

mach_vm_address_t MachO::getSymbolAddressByName(char *symbolname)
{
	Symbol *symbol = this->getSymbolByName(symbolname);

	if(!symbol)
		return 0;

	return symbol->getAddress();
}

off_t MachO::addressToOffset(mach_vm_address_t address)
{
	struct mach_header_64 *mh = this->getMachHeader();

	uint64_t current_offset = sizeof(struct mach_header_64);

	for(uint32_t i = 0; i < mh->ncmds; i++)
	{
		struct load_command *load_command = reinterpret_cast<struct load_command*>((*this)[current_offset]);

		uint32_t cmdtype = load_command->cmd;
		uint32_t cmdsize = load_command->cmdsize;

		if(cmdtype == LC_SEGMENT_64)
		{
			struct segment_command_64 *segment_command = reinterpret_cast<struct segment_command_64*>(load_command);

			uint64_t vmaddr = segment_command->vmaddr;
			uint64_t vmsize = segment_command->vmsize;

			uint64_t fileoff = segment_command->fileoff;
			uint64_t filesize = segment_command->filesize;

			uint64_t sect_offset = current_offset + sizeof(struct segment_command_64);

			for(int j = 0; j < segment_command->nsects; j++)
			{
				struct section_64 *section = reinterpret_cast<struct section_64*>((*this)[sect_offset]);

				uint64_t sectaddr = section->addr;
				uint64_t sectsize = section->size;

				uint64_t offset = section->offset;

				if(address >= sectaddr && address <= sectaddr + sectsize)
				{
					return offset == 0 ? 0 : offset + (address - sectaddr);
				}

				sect_offset += sizeof(struct section_64);
			}
		}

		current_offset += cmdsize;
	}

	return 0;
}

mach_vm_address_t MachO::offsetToAddress(off_t offset)
{
	struct mach_header_64 *mh = this->getMachHeader();

	uint64_t current_offset = sizeof(struct mach_header_64);

	for(uint32_t i = 0; i < mh->ncmds; i++)
	{
		struct load_command *load_command = reinterpret_cast<struct load_command*>((*this)[current_offset]);

		uint32_t cmdtype = load_command->cmd;
		uint32_t cmdsize = load_command->cmdsize;

		if(cmdtype == LC_SEGMENT_64)
		{
			struct segment_command_64 *segment_command = reinterpret_cast<struct segment_command_64*>(load_command);

			uint64_t vmaddr = segment_command->vmaddr;
			uint64_t vmsize = segment_command->vmsize;

			uint64_t fileoff = segment_command->fileoff;
			uint64_t filesize = segment_command->filesize;

			if(offset >= fileoff && offset <= fileoff + filesize)
			{
				return vmaddr + (offset - fileoff);
			}
		}

		current_offset += cmdsize;
	}

	return 0;
}

void* MachO::addressToPointer(mach_vm_address_t address)
{
	return reinterpret_cast<void*>(reinterpret_cast<mach_vm_address_t>(buffer + this->addressToOffset(address)));
}

mach_vm_address_t MachO::getBufferAddress(mach_vm_address_t address)
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

Segment* MachO::getSegment(char *segmentname)
{
	for(uint32_t i = 0; i < this->getSegments()->getSize(); i++)
	{
		Segment *segment = this->getSegments()->get(i);

		if(strcmp(segment->getSegmentName(), segmentname) == 0 ||
		   strncmp(segment->getSegmentName(), segmentname, strlen(segmentname)) == 0)
		{
			return segment;
		}
	}

	return NULL;
}

Section* MachO::getSection(char *segmentname, char *sectionname)
{
	for(uint32_t i = 0; i < this->getSegments()->getSize(); i++)
	{
		Segment *segment = this->getSegments()->get(i);

		if(strcmp(segment->getSegmentName(), segmentname) == 0 ||
			strncmp(segment->getSegmentName(), segmentname, strlen(segmentname)) == 0)
		{
			for(uint32_t j = 0; j < segment->getSections()->getSize(); j++)
			{
				Section *section = segment->getSections()->get(j);

				if(strcmp(section->getSectionName(), sectionname) == 0 ||
				   strncmp(section->getSectionName(), sectionname, strlen(sectionname)) == 0)
				{
					return section;
				}
			}
		}
	}

	return NULL;
}

Segment* MachO::segmentForAddress(mach_vm_address_t address)
{
	for(uint32_t i = 0; i < this->getSegments()->getSize(); i++)
	{
		Segment *segment = this->getSegments()->get(i);

		if(address >= segment->getAddress() && address <= segment->getAddress() + segment->getSize())
		{
			return segment;
		}
	}

	return NULL;
}

Section* MachO::sectionForAddress(mach_vm_address_t address)
{
	Segment *segment = this->segmentForAddress(address);

	if(!segment)
		return NULL;

	for(uint32_t i = 0; i < segment->getSections()->getSize(); i++)
	{
		Section *section = segment->getSections()->get(i);

		if(address >= section->getAddress() && address <= section->getAddress() + section->getSize())
		{
			return section;
		}
	}

	return NULL;
}

Segment* MachO::segmentForOffset(off_t offset)
{
	mach_vm_address_t address = this->offsetToAddress(offset);

	if(!address)
		return NULL;

	return this->segmentForAddress(address);
}

Section* MachO::sectionForOffset(off_t offset)
{
	mach_vm_address_t address = this->offsetToAddress(offset);

	if(!address)
		return NULL;

	return this->sectionForAddress(address);
}

void MachO::parseSymbolTable(struct nlist_64 *symtab, uint32_t nsyms, char *strtab, size_t strsize)
{
	for(int i = 0; i < nsyms; i++)
	{
		Symbol *symbol;

		struct nlist_64 *nl = &symtab[i];

		char *name;

		mach_vm_address_t address;

		name = &strtab[nl->n_strx];

		address = nl->n_value;

		symbol = new Symbol(this, nl->n_type & N_TYPE, name, address, this->addressToOffset(address), this->segmentForAddress(address), this->sectionForAddress(address));

		this->symbolTable->addSymbol(symbol);
	}

	MAC_RK_LOG("MacRK::MachO::%u syms!\n", nsyms);
}

void MachO::parseLinkedit()
{

}

bool MachO::parseLoadCommands()
{
	return true;
}

void MachO::parseFatHeader()
{
	struct fat_header *fat_header = reinterpret_cast<struct fat_header*>(this->getMachHeader());

	struct fat_arch *arch = reinterpret_cast<struct fat_arch*>(reinterpret_cast<uint8_t*>(fat_header) + sizeof(struct fat_header));

	uint32_t nfat_arch = OSSwapBigToHostInt32(fat_header->nfat_arch);

	for(uint32_t i = 0; i < nfat_arch; i++)
	{
		uint32_t cputype;
		uint32_t cpusubtype;

		uint32_t offset;

		cputype = OSSwapBigToHostInt32(arch->cputype);
		cpusubtype = OSSwapBigToHostInt32(arch->cpusubtype);

		offset = OSSwapBigToHostInt32(arch->offset);

	#ifdef __x86_64__
		#define NATIVE_CPU_TYPE   CPU_TYPE_X86_64
	#endif

	#ifdef __arm64__
		#define NATIVE_CPU_TYPE   CPU_TYPE_ARM64
	#endif

		if(cputype == NATIVE_CPU_TYPE)
		{
			struct mach_header_64 *mh = reinterpret_cast<struct mach_header_64*>(reinterpret_cast<uint8_t*>(fat_header) + offset);

			this->header = mh;
			this->buffer = reinterpret_cast<char*>(mh);

			this->parseHeader();
		}

	}
}

void MachO::parseHeader()
{
	struct mach_header_64 *mh = this->getMachHeader();

	uint32_t magic = mh->magic;

	if(magic == FAT_CIGAM)
	{

	} else if(magic == MH_MAGIC_64)
	{
		getAslrSlide();

		this->size = this->getSize();

		this->parseLoadCommands();
	}
}


void MachO::parseMachO()
{
	this->parseHeader();
}