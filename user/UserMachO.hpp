#ifndef _USER_MACHO_HPP__
#define _USER_MACHO_HPP__

#include "MachO.hpp"
#include "Array.hpp"
#include "SymbolTable.hpp"

#include "ObjC.hpp"
#include "Swift.hpp"

#include "Dyld.hpp"

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
	class CodeSignature
	{
		public:
			explicit CodeSignature(UserMachO *macho, struct linkedit_data_command *cmd) { this->macho = macho; this->cmd = cmd; this->parseCodeSignature(); }

			static CodeSignature* codeSignatureWithLinkedit(UserMachO *macho, struct linkedit_data_command *cmd);

			UserMachO* getMachO() { return macho; }

			struct linkedit_data_command* getLoadCommand() { return cmd; }

			SuperBlob* getSuperBlob() { return superBlob; }

			code_directory_t getCodeDirectory() { return codeDirectory; }

			char* getEntitlements() { return entitlements; }

			bool verifyCodeSlot(uint8_t *blob, size_t size, bool sha256, char *signature, size_t sigsize);

			bool compareHash(uint8_t *hash1, uint8_t *hash2, size_t hashSize);

			uint8_t* computeHash(bool sha256, uint8_t *blob, size_t size);

			bool parseCodeSignature();

		private:
			UserMachO *macho;

			struct linkedit_data_command *cmd;

			SuperBlob *superBlob;

			code_directory_t codeDirectory;

			char *entitlements;
	};

	class UserMachO : public MachO
	{
		public:
			UserMachO() { this-> task = NULL; this->file_path = NULL; }
			UserMachO(const char *path);

			~UserMachO() { }

			virtual void withTask(xnu::Task *task);
			virtual void withFilePath(const char *path);

			virtual void withBuffer(char *buffer);
			virtual void withBuffer(char *buffer, off_t slide);
			virtual void withBuffer(char *buffer, uint64_t size);
			
			virtual void withBuffer(mach_vm_address_t base, char *buffer, off_t slide);
			virtual void withBuffer(mach_vm_address_t base, char *buffer, off_t slide, bool is_dyld_cache);
			
			virtual void withBuffer(mrk::UserMachO *libobjc, mach_vm_address_t base, char *buffer, off_t slide);

			char* getFilePath() { return this->dyld ? this->dyld->getMainImagePath() : this->file_path; }

			bool isDyldCache() { return is_dyldCache; }

			void setIsDyldCache(bool isDyldCache) { this->is_dyldCache = isDyldCache; }

			UserMachO* getObjectiveCLibrary() { return libobjc; }

			ObjectiveC::ObjCData* getObjCMetadata() { return objc; }

			bool isObjectiveCLibrary() { return is_libobjc; }

			void setIsObjectiveCLibrary(bool is_libobjc) { this->is_libobjc = is_libobjc; }

			void setObjectiveCLibrary(UserMachO* libobjc) { this->libobjc = libobjc; }

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

			void parseCodeSignature(struct linkedit_data_command *cmd) { codeSignature = CodeSignature::codeSignatureWithLinkedit(this, cmd); }
			
			void parseObjC()
			{
				this->objc = ObjectiveC::parseObjectiveC(this);
			}

			void parseSwift()
			{
				this->swift = Swift::parseSwift(this);
			}

			uint8_t* operator[](uint64_t index) { return this->getOffset(index); }

		private:
			xnu::Task *task;

			mrk::UserMachO *libobjc;

			dyld::Dyld *dyld;

			mach_vm_address_t dyld_base;
			mach_vm_address_t dyld_shared_cache;

			mrk::CodeSignature *codeSignature;

			ObjectiveC::ObjCData *objc;
			Swift::SwiftMetadata *swift;

			char *file_path;

			bool is_dyldCache;
			bool is_libobjc;

			uint64_t readUleb128(uint8_t *start, uint8_t *end, uint32_t *idx);
			int64_t  readSleb128(uint8_t *start, uint8_t *end, uint32_t *idx);
	};
}

#endif
