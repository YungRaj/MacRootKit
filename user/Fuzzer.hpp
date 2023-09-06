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

			explicit RawBinary(const char *path, const char *mapFile);

			uintptr_t getBase() { return base; }

			char* getMapFile() { return mapFile; }

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

			void populateSymbolsFromLinkerMap();
			void populateSegmentsFromLinkerMap();

		private:
			uintptr_t base;

			char *mapFile;

			Array<SymbolRaw*> symbols;
			Array<SegmentRaw*> segments;
	};

	template <typename T>
	concept ClassType = std::is_class_v<T>;

	template <typename T>
	concept FundamentalType = std::is_fundamental_v<T>;

	template <typename T>
	concept PodType = std::is_pod_v<T>;

	template<typename T>
	concept IntegralType = std::is_integral_v<T>;

	template<typename T>
	concept PointerType = std::is_pointer_v<T>;

	template<typename T>
	concept IntegralOrPointerType = IntegralType<T> || PointerType<T>;

	template<typename T>
	concept CastableType = FundamentalType<T> || PodType<T> || IntegralType<T> || PointerType<T>;

	struct FuzzBinary
	{
		const char *path;

		void *base;
		void *originalBase;

		size_t size;

		template<typename T>
		concept BinaryFormat = std::is_baseof<MachO, T> || std::is_same_v<T, MachO*> || std::is_same_v<T, RawBinary>;

		union
		{
			/* Support MachO and Raw Binary */
			MachO *macho;

			RawBinary *raw;

			/* Support ELFs and PE32 binaries later */
			/* This union constrains the Binary Format types */
		} binary;

		static_assert(BinaryFormat<decltype(binary)>, "All types in the union must satisfy the BinaryFormat concept");

		template<typename T>
		T operator[](uint64_t index) requires CastableType<T> { return reinterpret_cast<T>((uint8_t*) base + index); }
		
		const char* getPath() { return path; }

		template<typename T> requires IntegralOrPointerType<T>
		T getBase()
		{
			return reinterpret_cast<T>(base);
		}

		template<typename T> requires IntegralOrPointerType<T>
		T getOriginalBase()
		{
			return reinterpret_cast<T>(base);
		}

		template<typename Sym, typename Binary>
		Sym getSymbol(char *symbolname) requires requires (Sym sym, Binary bin) {
			{ bin->getSymbol(); sym->getName(); sym->getAddress();  }
		}
		{
			static_assert(BinaryFormat<Binary>,
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

		template<typename Seg, typename Binary>
		Seg getSegment(char *segname) requires requires (Seg seg, Binary bin) {
			{ bin->getSegment(); seg->getName(); seg->getAddress(); }
		}
		{
			static_assert(BinaryFormat<Binary>,
		                  "Unsupported type for FuzzBinary:getSegment()");

		    if constexpr (std::is_base_of<MachO, Binary>::value)
		    {
		        return dynamic_cast<Seg>(this->fuzzBinary->binary.macho->getSegment(segname));
		    }

		    if constexpr (std::is_same_v<Binary, MachO*>)
		    {
		        return this->fuzzBinary->binary.macho->getSegment(segname);
		    }

		    if constexpr (std::is_same_v<Binary, RawBinary*>)
		    {
		        return this->fuzzBinary->binary.raw->getSegment(segname);
		    }

		    return NULL;
		}
	};

	template <typename T>
	concept FuzzableType = ClassType<T> || FundamentalType<T> || PodType<T> || IntegralOrPointerType<T>;

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
		    static_assert(BinaryFormat<T>,
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

		char* getMapFile() { return mapFile; }

		template <typename CpuType>
		char* getMachOFromFatHeader(char *file_data);

		bool mapSegmentsFromRawBinary(char *file_data, char *mapFile);
		bool mapSegmentsFromMachO(char *file_data);

		void getMappingInfoForMachO(char *file_data, size_t *size, uintptr_t *load_addr);

		void updateSegmentLoadCommandsForNewLoadAddress(char *file_data, uintptr_t newLoadAddress, uintptr_t oldLoadAddress);
		void updateSymbolTableForMappedMachO(char *file_data, uintptr_t newLoadAddress, uintptr_t oldLoadAddress);

		void loadBinary(const char *path, const char *mapFile);
		void loadKernel(const char *path, off_t slide);
		void loadKernelExtension(const char *path);

		void loadKernelMachO(const char *kernelPath, uintptr_t *loadAddress, size_t *loadSize, uintptr_t *oldLoadAddress);

		void populateSymbolsFromMapFile(const char *mapFile);

		template <typename T>
		void mutate(T data) requires FuzzableType<T>;

		private:
			xnu::Kernel *kernel;

			xnu::KDKInfo *kdk;

			struct FuzzBinary *fuzzBinary;

			Loader *loader;

			const char *mapFile;
	};
};

#endif