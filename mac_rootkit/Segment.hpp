#ifndef __SEGMENT_HPP_
#define __SEGMENT_HPP_

#include <mach/mach_types.h>
#include <sys/types.h>

#include "Array.hpp"
#include "Section.hpp"

#include "mach-o.h"

#include "Log.hpp"

class Segment
{
	public:
		Segment(struct segment_command_64 *segment_command)
			: segment(segment_command),
			  initprot(segment_command->initprot),
			  maxprot(segment_command->maxprot),
			  address(segment_command->vmaddr),
			  size(segment_command->vmsize),
			  fileoffset(segment_command->fileoff),
			  filesize(segment_command->filesize)
		{
			name = new char[strlen(segment_command->segname) + 1];
			
			strlcpy(this->name, segment_command->segname, strlen(segment_command->segname) + 1);

			this->populateSections();
		}

		~Segment()
		{
			delete name;

			for(int i = 0; i < sections.getSize(); i++)
			{
				Section *section = sections.get(i);

				delete section;
			}
		}

		struct segment_command_64* getSegmentCommand() { return segment; }

		vm_prot_t getProt() { return maxprot; }

		char* getSegmentName() { return name; }

		mach_vm_address_t getAddress() { return address; }

		size_t getSize() { return size; }

		off_t getFileOffset() { return fileoffset; }

		size_t getFileSize() { return filesize; }

		std::Array<Section*>* getSections() { return &sections; }

		Section* getSection(char *sectname)
		{
			for(int i = 0; i < sections.getSize(); i++)
			{
				Section *section = sections.get(i);

				if(strcmp(section->getSectionName(), sectname) == 0 ||
				   strncmp(section->getSectionName(), sectname, strlen(sectname)) == 0)
				{
					return section;
				}
			}

			return NULL;
		}

		void addSection(Section *section) { sections.add(section); }

		void populateSections()
		{
			struct segment_command_64 *segment = this->segment;

			uint32_t nsects = segment->nsects;
			uint32_t offset = sizeof(struct segment_command_64);

			for(int32_t i = 0; i < nsects; i++)
			{
				struct section_64 *sect = reinterpret_cast<struct section_64*>((uint8_t*) segment + offset);

				Section *section = new Section(sect);

				this->addSection(section);

				offset += sizeof(struct section_64);
			}
		}

	private:
		struct segment_command_64 *segment;

		std::Array<Section*> sections;

		char *name;

		mach_vm_address_t address;

		size_t size;

		off_t fileoffset;

		size_t filesize;

		vm_prot_t	maxprot;
		vm_prot_t	initprot;
};

#endif