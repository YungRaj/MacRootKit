#include "KernelMachO.hpp"

#include "Kernel.hpp"

KernelMachO::KernelMachO(Kernel *kernel)
{
	this->kernel = kernel;
#ifdef __x86_64__
	this->kernel_collection = Kernel::findKernelCollection();
	this->kernel_cache = 0;
#elif __arm64__
	this->kernel_cache = Kernel::findKernelCache();
	this->kernel_collection = 0;
#endif
}

KernelMachO::~KernelMachO()
{
}

Kext* KernelMachO::kextLoadedAt(Kernel *kernel, mach_vm_address_t address)
{
	mrk::MacRootKit *rootkit = kernel->getRootKit();

	return rootkit->getKextByAddress(address);
}

Kext* KernelMachO::kextWithIdentifier(Kernel *kernel, char *kextname)
{
	mrk::MacRootKit *rootkit = kernel->getRootKit();

	return rootkit->getKextByIdentifier(kextname);
}

void KernelMachO::parseLinkedit()
{
	MachO::parseLinkedit();
}

bool KernelMachO::parseLoadCommands()
{
	struct mach_header_64 *mh = this->getMachHeader();

	size_t file_size;

	this->size = this->getSize();

	file_size = this->size;

	uint8_t *q = reinterpret_cast<uint8_t*>(mh) + sizeof(struct mach_header_64);

	uint32_t current_offset = sizeof(struct mach_header_64);

	for(uint32_t i = 0; i < mh->ncmds; i++)
	{
		struct load_command *load_command = reinterpret_cast<struct load_command*>((*this)[current_offset]);

		uint32_t cmdtype = load_command->cmd;
		uint32_t cmdsize = load_command->cmdsize;

		if(cmdsize > mh->sizeofcmds - ((uintptr_t) load_command - (uintptr_t)(mh + 1)))
			return false;

		switch(cmdtype)
		{
			case LC_SEGMENT_64:
			{
				;
				struct segment_command_64 *segment_command = reinterpret_cast<struct segment_command_64*>(load_command);

				uint32_t nsects = segment_command->nsects;
				uint32_t sect_offset = current_offset + sizeof(struct segment_command_64);

				if(segment_command->fileoff > this->size || segment_command->filesize > this->size - segment_command->fileoff)
					return false;

				char buffer1[128];
				char buffer2[128];

				snprintf(buffer1, 128, "0x%08llx", segment_command->vmaddr);
				snprintf(buffer2, 128, "0x%08llx", segment_command->vmaddr + segment_command->vmsize);

				vm_prot_t maxprot = segment_command->maxprot;

				char r[24];
				memcpy(r, ((maxprot & VM_PROT_READ) ? "read" : "-"), 24);

				char w[24];
				memcpy(w, ((maxprot & VM_PROT_WRITE) ? "write" : "-"), 24);
				
				char x[24];
				memcpy(x, ((maxprot & VM_PROT_EXECUTE) ? "execute" : "-"), 24);

				printf("MacRK::LC_SEGMENT_64 at 0x%llx (%s/%s/%s) - %s %s to %s \n", segment_command->fileoff,
																						r,
																						w,
																						x,
																						segment_command->segname,
																						buffer1,
																						buffer2
																						); 
				
				
				if (!strcmp(segment_command->segname, "__LINKEDIT"))
				{
					linkedit = reinterpret_cast<uint8_t *>(segment_command->vmaddr);
					linkedit_off = segment_command->fileoff;
					linkedit_size = segment_command->filesize;
				}

				int j = 0;

				if(nsects * sizeof(struct section_64) + sizeof(struct segment_command_64) > cmdsize)
					return false;

				Segment *segment = new Segment(segment_command);

				for(j = 0; j < nsects; j++)
				{
					struct section_64 *section = reinterpret_cast<struct section_64*>((*this)[sect_offset]);

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

				this->segments.add(segment);

				break;
			}

			case LC_SYMTAB:
			{
				;
				struct symtab_command *symtab_command = reinterpret_cast<struct symtab_command*>(load_command);

				struct nlist_64 *symtab;
				uint32_t nsyms;

				char *strtab;
				uint32_t strsize;

				if(symtab_command->stroff > this->size || symtab_command->symoff > this->size || symtab_command->nsyms > (this->size - symtab_command->symoff) / sizeof(struct nlist_64))
					return false;

				MAC_RK_LOG("MacRK::LC_SYMTAB\n");
				MAC_RK_LOG("MacRK::\tSymbol Table is at offset 0x%x (%u) with %u entries \n",symtab_command->symoff,symtab_command->symoff,symtab_command->nsyms);
				MAC_RK_LOG("MacRK::\tString Table is at offset 0x%x (%u) with size of %u bytes\n",symtab_command->stroff,symtab_command->stroff,symtab_command->strsize);

				if(this->kernel_cache)
				{
					symtab = reinterpret_cast<struct nlist_64*>(this->kernel_cache + symtab_command->symoff);
					nsyms = symtab_command->nsyms;

					strtab = reinterpret_cast<char*>(this->kernel_cache + symtab_command->stroff);
					strsize = symtab_command->strsize;

					char buffer1[128];
					char buffer2[128];

					snprintf(buffer1, 128, "0x%llx", (uint64_t) symtab);
					snprintf(buffer2, 128, "0x%llx", (uint64_t) strtab);

					MAC_RK_LOG("MacRK::\tSymbol Table address = %s\n", buffer1);
					MAC_RK_LOG("MacRK::\tString Table address = %s\n", buffer2);
					
				} else if(this->kernel_collection)
				{
					symtab = reinterpret_cast<struct nlist_64*>(linkedit + (symtab_command->symoff - linkedit_off));
					nsyms = symtab_command->nsyms;

					strtab = reinterpret_cast<char*>(linkedit + (symtab_command->stroff - linkedit_off));
					strsize = symtab_command->strsize;

				} else
				{
					symtab = NULL;
					nsyms = 0;

					strtab = NULL;
					strsize = 0;
				}

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

				uint32_t dataoff = linkedit->dataoff;
				uint32_t datasize = linkedit->datasize;

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

				uint32_t dataoff = linkedit->dataoff;
				uint32_t datasize = linkedit->datasize;

				MAC_RK_LOG("MacRK::LC_DATA_IN_CODE\n");
				MAC_RK_LOG("MacRK::\tOffset = 0x%x Size = 0x%x\n", dataoff, datasize);

				break;
			}
		}

		current_offset += cmdsize;
	}

	return true;
}

void KernelMachO::parseMachO()
{
	MachO::parseMachO();
}