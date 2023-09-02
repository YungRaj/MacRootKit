#ifndef __FUZZER_HPP_
#define __FUZZER_HPP_

#include <type_traits>

class MachO;

namespace Fuzzer
{
	class Loader;
	class Module;

	struct RawBinary
	{
		struct SegmentRaw
		{
			public:
				SegmentRaw(uintptr_t address, size_t size, int prot)
				{
					this->address = address;
					this->size = size;
					this->prot = prot;
				}

				uintptr_t getAddress() { return address; }

				size_t getSize() { return size; }

				int getProt() { return prot; }
			private:
				uintptr_t address;

				size_t size;

				int prot;

		};

		struct SymbolRaw
		{
			public:
				SymbolRaw(const char *name, uintptr_t address, int type)
				{
					this->name = name;
					this->address = address;
					this->type = type;
				}

				const char* getName() { return name; }

				uintptr_t getAddress() { return address; }

				int getType() { return type; }

			private:
				const char *name;

				uintptr_t address;

				int type;
		};

		char* getSymbolsFile() { return symbolsFile; }

		SymbolRaw* getSymbol(const char *name)
		{
			for(int i = 0; i < symbols.getSize(); i++)
			{
				SymbolRaw *sym = symbols.get(i);

				if(strcmp(name, symbol->getName()) == 0)
					return sym;
			}

			return NULL;
		}

		SegmentRaw* getSegment(const char *name)
		{
			for(int i = 0; i < segments.getSize(); i++)
			{
				SegmentRaw *seg = segments.get(i);

				if(strcmp(name, symbol->getName()) == 0)
					return seg;
			}

			return NULL;
		}

		char *symbolsFile;

		Array<SymbolRaw*> symbols;
		Array<SegmentRaw*> segments;
	};

	struct FuzzBinary
	{
		const char *path;

		void *base;

		size_t size;

		union
		{
			MachO *macho;

			RawBinary *raw;
		} binary;
	};

	class Harness
	{
		explicit Harness();

		~Harness();

		struct FuzzBinary* getFuzzBinary() { return fuzzBinary; }

		template<typename T>
		T getBinary() {
		    static_assert(std::is_same_v<T, MachO*> || std::is_same_v<T, RawBinary*>,
		                  "Unsupported type for Harness::getBinary()");

		    if constexpr (std::is_same_v<T, MachO*>) {
		        return this->fuzzBinary->binary.macho;
		    } else {
		        return this->fuzzBinary->binary.raw;
		    }
		}

		Loader* getLoader() { return loader; }

		void loadBinary(const char *path, const char *symbolsFile);
		void loadKernel(const char *path, uintptr_t base, off_t slide);
		void loadMachO(const char *path);

		void populateSymbols(const char *symbolsFile);

		private:
			struct FuzzBinary *fuzzBinary;

			Loader *loader;

			const char *symbolsFilePath;
	};
};

#endif