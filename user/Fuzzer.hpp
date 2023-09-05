#ifndef __FUZZER_HPP_
#define __FUZZER_HPP_

#include <type_traits>

extern "C"
{
	#include <stdio.h>
};

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
				SegmentRaw(uintptr_t address, size_t size, int prot) : address(address), size(size), prot(prot) { }

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
				SymbolRaw(const char *name, uintptr_t address, int type) : name(name), address(address), type(type) { }

				const char* getName() { return name; }

				uintptr_t getAddress() { return address; }

				int getType() { return type; }

				bool isUndefined() { return false; }

				bool isExternal() { return false; }

			private:
				const char *name;

				uintptr_t address;

				int type;
		};

		public:

			explicit RawBinary(const char *path, const char *symbolsFile);

			static RawBinary* rawBinaryFromSymbolsFile(const char *path, const char *symbolsFile);

			uintptr_t getBase() { return base; }

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

		private:
			uintptr_t base;

			char *symbolsFile;

			Array<SymbolRaw*> symbols;
			Array<SegmentRaw*> segments;
	};

	struct FuzzBinary
	{
		const char *path;

		void *base;
		void *originalBase;

		size_t size;

		union
		{
			MachO *macho;

			RawBinary *raw;
		} binary;

		template<typename Sym, typename Binary>
		Sym getSymbol(char *symbolname) requires requires (Sym sym) {
			{ sym->getSymbol(); }
		}
		{
			static_assert(std::is_same_v<Binary, MachO*> || std::is_same_v<Binary, RawBinary*>,
		                  "Unsupported type for FuzzBinary:getSymbol()");

		    if constexpr (std::is_base_of<MachO, Binary>::value)
		    {
		        return dynamic_cast<Sym>(this->fuzzBinary->binary.macho->getSymbol(symbolname));
		    }

		    if constexpr (std::is_same_v<Binary, MachO*>)
		    {
		        return this->fuzzBinary->binary.macho->getSymbol(symbolname);
		    }

		    if constexpr (std::is_same_v<Binary, RawBinary*>)
		    {
		        return this->fuzzBinary->binary.raw->getSymbol(symbolname);
		    }

		    return NULL;
		}
	};

	template <typename T>
	struct FuzzableType
	{
	    static constexpr bool value =
	        std::is_class_v<T> || std::is_fundamental_v<T> || std::is_pod_v<T>;
	};

	class Harness
	{
		explicit Harness(xnu::Kernel *kernel);

		~Harness();

		struct FuzzBinary* getFuzzBinary() { return fuzzBinary; }

		template<typename Sym>
		Array<Sym>* getSymbols() requires requires (Sym sym) {
			{ sym->getName() };
			{ sym->getAddress() };
		}

		template<typename T>
		T getBinary()
		{
		    static_assert(std::is_same_v<T, MachO*> || std::is_same_v<T, RawBinary*>,
		                  "Unsupported type for Module::getBinary()");

		    if constexpr (std::is_base_of<MachO, T>::value)
		    {
		        return dynamic_cast<T>(this->fuzzBinary->binary.macho);
		    }

		    if constexpr (std::is_same_v<T, MachO*>)
		    {
		        return this->fuzzBinary->binary.macho;
		    }

		    if constexpr (std::is_same_v<T, RawBinary*>)
		    {
		        return this->fuzzBinary->binary.raw;
		    }

		    return NULL;
		}

		Loader* getLoader() { return loader; }

		char* getMachOFromFatHeader(char *file_data);

		bool mapSegmentsFromRawBinary(char *file_data, char *symbolsFile);
		bool mapSegmentsFromMachO(char *file_data);

		void getMappingInfoForMachO(char *file_data, size_t *size, uintptr_t *load_addr);

		void updateSegmentLoadCommandsForNewLoadAddress(char *file_data, uintptr_t newLoadAddress, uintptr_t oldLoadAddress);
		void updateSymbolTableForMappedMachO(char *file_data, uintptr_t newLoadAddress, uintptr_t oldLoadAddress);

		void loadBinary(const char *path, const char *symbolsFile);
		void loadKernel(const char *path, off_t slide);
		void loadKernelExtension(const char *path);

		void loadKernelMachO(const char *kernelPath, uintptr_t *loadAddress, size_t *loadSize, uintptr_t *oldLoadAddress);

		void populateSymbolsFromSymbolsFile(const char *symbolsFile);

		template <typename T>
		void mutate(T data) requires FuzzableType<T>;

		private:
			xnu::Kernel *kernel;

			xnu::KDKInfo *kdk;

			struct FuzzBinary *fuzzBinary;

			Loader *loader;

			const char *symbolsFilePath;
	};
};

#endif