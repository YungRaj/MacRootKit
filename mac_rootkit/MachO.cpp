#include "MachO.hpp"
#include "Log.hpp"

MachO::MachO()
{

}

MachO::~MachO()
{
	
}

void MachO::initWithBase(xnu::Mach::VmAddress machoBase, Offset slide)
{
	base = machoBase;
	aslr_slide = slide;
	buffer = reinterpret_cast<char*>(base);
	header = reinterpret_cast<xnu::Macho::Header64*>(buffer);
	symbolTable = new SymbolTable();

	this->parseMachO();
}

Size MachO::getSize()
{
	Size file_size = 0;

	xnu::Macho::Header64 *mh = this->getMachHeader();

	UInt8 *q = reinterpret_cast<UInt8*>(mh) + sizeof(xnu::Macho::Header64);

	for(UInt32 i = 0; i < mh->ncmds; i++)
	{
		struct load_command *load_command = reinterpret_cast<struct load_command*>(q);

		UInt32 cmd = load_command->cmd;
		UInt32 cmdsize = load_command->cmdsize;

		if(cmd == LC_SEGMENT_64)
		{
			struct segment_command_64 *segment_command = reinterpret_cast<struct segment_command_64*>(load_command);

			UInt64 vmaddr = segment_command->vmaddr;
			UInt64 vmsize = segment_command->vmsize;

			UInt64 fileoff = segment_command->fileoff;
			UInt64 filesize = segment_command->filesize;

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

Symbol* MachO::getSymbolByAddress(xnu::Mach::VmAddress address)
{
	return this->symbolTable->getSymbolByAddress(address);
}

xnu::Mach::VmAddress MachO::getSymbolAddressByName(char *symbolname)
{
	Symbol *symbol = this->getSymbolByName(symbolname);

	if(!symbol)
		return 0;

	return symbol->getAddress();
}

Offset MachO::addressToOffset(xnu::Mach::VmAddress address)
{
	xnu::Macho::Header64 *mh = this->getMachHeader();

	UInt64 current_offset = sizeof(xnu::Macho::Header64);

	for(UInt32 i = 0; i < mh->ncmds; i++)
	{
		struct load_command *load_command = reinterpret_cast<struct load_command*>((*this)[current_offset]);

		UInt32 cmdtype = load_command->cmd;
		UInt32 cmdsize = load_command->cmdsize;

		if(cmdtype == LC_SEGMENT_64)
		{
			struct segment_command_64 *segment_command = reinterpret_cast<struct segment_command_64*>(load_command);

			UInt64 vmaddr = segment_command->vmaddr;
			UInt64 vmsize = segment_command->vmsize;

			UInt64 fileoff = segment_command->fileoff;
			UInt64 filesize = segment_command->filesize;

			UInt64 sect_offset = current_offset + sizeof(struct segment_command_64);

			for(int j = 0; j < segment_command->nsects; j++)
			{
				struct section_64 *section = reinterpret_cast<struct section_64*>((*this)[sect_offset]);

				UInt64 sectaddr = section->addr;
				UInt64 sectsize = section->size;

				UInt64 offset = section->offset;

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

xnu::Mach::VmAddress MachO::offsetToAddress(Offset offset)
{
	xnu::Macho::Header64 *mh = this->getMachHeader();

	UInt64 current_offset = sizeof(xnu::Macho::Header64);

	for(UInt32 i = 0; i < mh->ncmds; i++)
	{
		struct load_command *load_command = reinterpret_cast<struct load_command*>((*this)[current_offset]);

		UInt32 cmdtype = load_command->cmd;
		UInt32 cmdsize = load_command->cmdsize;

		if(cmdtype == LC_SEGMENT_64)
		{
			struct segment_command_64 *segment_command = reinterpret_cast<struct segment_command_64*>(load_command);

			UInt64 vmaddr = segment_command->vmaddr;
			UInt64 vmsize = segment_command->vmsize;

			UInt64 fileoff = segment_command->fileoff;
			UInt64 filesize = segment_command->filesize;

			if(offset >= fileoff && offset <= fileoff + filesize)
			{
				return vmaddr + (offset - fileoff);
			}
		}

		current_offset += cmdsize;
	}

	return 0;
}

void* MachO::addressToPointer(xnu::Mach::VmAddress address)
{
	return reinterpret_cast<void*>(reinterpret_cast<xnu::Mach::VmAddress>(buffer + this->addressToOffset(address)));
}

xnu::Mach::VmAddress MachO::getBufferAddress(xnu::Mach::VmAddress address)
{
	xnu::Mach::VmAddress header = reinterpret_cast<xnu::Mach::VmAddress>(this->getMachHeader());

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
	for(UInt32 i = 0; i < this->getSegments().size(); i++)
	{
		Segment *segment = this->getSegments().at(i);

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
	for(UInt32 i = 0; i < this->getSegments().size(); i++)
	{
		Segment *segment = this->getSegments().at(i);

		if(strcmp(segment->getSegmentName(), segmentname) == 0 ||
			strncmp(segment->getSegmentName(), segmentname, strlen(segmentname)) == 0)
		{
			for(UInt32 j = 0; j < segment->getSections().size(); j++)
			{
				Section *section = segment->getSections().at(j);

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

Segment* MachO::segmentForAddress(xnu::Mach::VmAddress address)
{
	for(UInt32 i = 0; i < this->getSegments().size(); i++)
	{
		Segment *segment = this->getSegments().at(i);

		if(address >= segment->getAddress() && address <= segment->getAddress() + segment->getSize())
		{
			return segment;
		}
	}

	return NULL;
}

Section* MachO::sectionForAddress(xnu::Mach::VmAddress address)
{
	Segment *segment = this->segmentForAddress(address);

	if(!segment)
		return NULL;

	for(UInt32 i = 0; i < segment->getSections().size(); i++)
	{
		Section *section = segment->getSections().at(i);

		if(address >= section->getAddress() && address <= section->getAddress() + section->getSize())
		{
			return section;
		}
	}

	return NULL;
}

Segment* MachO::segmentForOffset(Offset offset)
{
	xnu::Mach::VmAddress address = this->offsetToAddress(offset);

	if(!address)
		return NULL;

	return this->segmentForAddress(address);
}

Section* MachO::sectionForOffset(Offset offset)
{
	xnu::Mach::VmAddress address = this->offsetToAddress(offset);

	if(!address)
		return NULL;

	return this->sectionForAddress(address);
}

bool MachO::addressInSegment(xnu::Mach::VmAddress address, char *segmentname)
{
	Segment *segment = this->segmentForAddress(address);

	if(segment && strcmp(segment->getSegmentName(), segmentname) == 0)
	{
		xnu::Mach::VmAddress segmentAddress = segment->getAddress();

		Size segmentSize = segment->getSize();

		return address >= segmentAddress && address < segmentAddress + segmentSize;
	}

	return false;
}

bool MachO::addressInSection(xnu::Mach::VmAddress address, char *segmentname, char *sectname)
{
	Section *section;

	if(this->addressInSegment(address, segmentname))
	{
		section = this->sectionForAddress(address);

		if(section && strcmp(section->getSectionName(), sectname) == 0)
		{
			xnu::Mach::VmAddress sectionAddress = section->getAddress();

			Size sectionSize = section->getSize();

			return address >= sectionAddress && address < sectionAddress + sectionSize;
		}

		return false;
	}

	return false;
}

void MachO::parseSymbolTable(xnu::Macho::Nlist64 *symtab, UInt32 nsyms, char *strtab, Size strsize)
{
	for(int i = 0; i < nsyms; i++)
	{
		Symbol *symbol;

		xnu::Macho::Nlist64 *nl = &symtab[i];

		char *name;

		xnu::Mach::VmAddress address;

		name = &strtab[nl->n_strx];

		address = nl->n_value;

		symbol = new Symbol(this, nl->n_type & N_TYPE, name, address, this->addressToOffset(address), this->segmentForAddress(address), this->sectionForAddress(address));

		this->symbolTable->addSymbol(symbol);

		MAC_RK_LOG("MacRK::Symbol %s 0x%llx\n", name, address);
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

	struct fat_arch *arch = reinterpret_cast<struct fat_arch*>(reinterpret_cast<UInt8*>(fat_header) + sizeof(struct fat_header));

	UInt32 nfat_arch = OSSwapBigToHostInt32(fat_header->nfat_arch);

	for(UInt32 i = 0; i < nfat_arch; i++)
	{
		UInt32 cputype;
		UInt32 cpusubtype;

		UInt32 offset;

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
			xnu::Macho::Header64 *mh = reinterpret_cast<xnu::Macho::Header64*>(reinterpret_cast<UInt8*>(fat_header) + offset);

			this->header = mh;
			this->buffer = reinterpret_cast<char*>(mh);

			this->parseHeader();
		}

	}
}

void MachO::parseHeader()
{
	xnu::Macho::Header64 *mh = this->getMachHeader();

	UInt32 magic = mh->magic;

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