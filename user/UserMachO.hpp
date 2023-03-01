#ifndef _USER_MACHO_HPP__
#define _USER_MACHO_HPP__

#include "MachO.hpp"
#include "Array.hpp"
#include "SymbolTable.hpp"

#include "ObjC.hpp"
#include "Swift.hpp"

#include "mach-o.h"

class Segment;
class Section;

namespace dyld
{
	class Dyld;
	class Library;
};

namespace ObjectiveC
{
	class ObjCData;
};

namespace xnu
{
	class Task;
};

namespace mrk
{
	class CodeSignature;

	class UserMachO : public MachO
	{
		public:
			UserMachO() { }
			UserMachO(const char *path);

			~UserMachO() { }

			virtual void initWithTask(xnu::Task *task);
			virtual void initWithFilePath(const char *path);
			virtual void initWithBuffer(char *buffer);
			virtual void initWithBuffer(char *buffer, off_t slide);

			virtual void initWithBuffer(mach_vm_address_t base, char *buffer, off_t slide);
			virtual void initWithBuffer(mach_vm_address_t base, char *buffer, off_t slide, bool is_dyld_cache);
			
			virtual void initWithBuffer(mrk::UserMachO *libobjc, mach_vm_address_t base, char *buffer, off_t slide);

			bool isDyldCache() { return is_dyldCache; }

			void setIsDyldCache(bool isDyldCache) { this->is_dyldCache = isDyldCache; }

			UserMachO* getObjectiveCLibrary() { return libobjc; }

			bool isObjectiveCLibrary() { return is_libobjc; }

			void setIsObjectiveCLibrary(bool is_libobjc) { this->is_libobjc = is_libobjc; }

			void setObjectiveCLibrary(UserMachO* libobjc) { this->libobjc = libobjc; }

			virtual void initWithBuffer(char *buffer, uint64_t size);

			static MachO* taskAt(mach_port_t task);
			static MachO* libraryLoadedAt(mach_port_t task, char *library);

			static uint64_t untagPacPointer(mach_vm_address_t base, enum dyld_fixup_t fixupKind, uint64_t ptr, bool *bind, bool *auth, uint16_t *pac, size_t *skip);

			bool isPointerInPacFixupChain(mach_vm_address_t ptr);

			mach_vm_address_t getBufferAddress(mach_vm_address_t address);

			virtual void parseMachO() override;

			virtual void parseHeader() override;

			virtual void parseFatHeader() override;

			virtual void parseSymbolTable(struct nlist_64 *symtab, uint32_t nsyms, char *strtab, size_t strsize) override;
			
			virtual void parseLinkedit() override;

			virtual bool parseLoadCommands() override;

			void parseCodeSignature(mrk::CodeSignature *signature);
			
			void parseObjC()
			{
				this->objc = ObjectiveC::parseObjectiveC(this);
			}

			void parseSwift()
			{
				this->swift = Swift::parseSwift(this);
			}

		private:
			xnu::Task *task;

			mrk::UserMachO *libobjc;

			dyld::Dyld *dyld;

			mach_vm_address_t dyld_base;
			mach_vm_address_t dyld_shared_cache;

			mrk::CodeSignature *codeSignature;

			ObjectiveC::ObjCData *objc;
			Swift::SwiftMetadata *swift;

			bool is_dyldCache;
			bool is_libobjc;

			uint64_t readUleb128(uint8_t *start, uint8_t *end, uint32_t *idx);
			int64_t  readSleb128(uint8_t *start, uint8_t *end, uint32_t *idx);
	};
}

#endif
