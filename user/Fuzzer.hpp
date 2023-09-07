#ifndef __FUZZER_HPP_
#define __FUZZER_HPP_

#include <type_traits>

extern "C"
{
	#include <stdio.h>
};

class MachO;
class KernelMachO;
class KextMachO;

namespace Fuzzer
{
	class Loader;
	class Module;

	struct RawBinary
	{
		struct SegmentRaw
		{
			public:
				explicit SegmentRaw(uintptr_t address, size_t size, int prot) : address(address), size(size), prot(prot) { }

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
				explicit SymbolRaw(const char *name, uintptr_t address, int type) : name(name), address(address), type(type) { }

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
	concept PointerToClassType = std::is_pointer_v<T> && std::is_class_v<std::remove_pointer_t<T>>;

	template <typename T>
	concept FundamentalType = std::is_fundamental_v<T>;

	template <typename T>
	concept ScalarType = std::is_scalar_v<T>;

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
		concept MachOFormat = std::is_baseof<MachO, std::remove_pointer_t<T>> || std::is_same_v<T, MachO*>;

		template<typename T>
		concept RawBinaryFormat = std::is_same_v<T, RawBinary*>;

		template<typename T>
		concept BinaryFormat = MachOFormat<T> || RawBinaryFormat<T>

		static_assert(MachOFormat<KernelMachO*>);
		static_assert(MachOFormat<KextMachO*>);

		static_assert(RawBinaryFormat<RawBinary*>);

		template <typename T>
		using GetSymbolReturnType = decltype(std::declval<T>()->getSymbol(nullptr));

		template <typename T>
		using GetSegmentReturnType = decltype(std::declval<T>()->getSegment(nullptr));

		union
		{
			/* We know the binary is a any of the below BinaryFormats */
			void *bin;

			/* Support MachO */
			MachO *macho;

			/* Support Raw Binary */
			RawBinary *raw;

			/* Support ELFs, PE32 and TE binaries later */

			/* Support EFI fuzzing on PE32/TE */
			/* Support Linux kernel fuzzing on ELFs */
			
			// PortableExecutable pe;
			// TerseExecutable te;
			// ELF elf;

			/* This union constrains the Binary Format types */
		} binary;

		static_assert(std::is_same_v<GetSymbolReturnType<MachO*>, Symbol*>);
		static_assert(std::is_same_v<GetSymbolReturnType<RawBinary*>, SymbolRaw*>);

		static_assert(std::is_same_v<GetSegmentReturnType<MachO*>, Segment*>);
		static_assert(std::is_same_v<GetSegmentReturnType<RawBinary*>, SegmentRaw*>);

		static_assert(BinaryFormat<decltype(binary.raw)> && BinaryFormat<decltype(binary.macho)>, "All types in the union must satisfy the BinaryFormat concept");

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
			{ sym->getName(); }
			{ sym->getAddress(); }
			{ std::is_same_v<GetSymbolReturnType<Binary>, Sym> };
		}
		{
			static_assert(BinaryFormat<Binary>,
		                  "Unsupported type for FuzzBinary:getSymbol()");

		    if constexpr (std::is_base_of<MachO, std::remove_pointer_t<Binary>>)
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
			{ seg->getName(); }
			{ seg->getAddress(); }
			{ std::is_same_v<GetSegmentReturnType<Binary>, Seg> }
		}
		{
			static_assert(BinaryFormat<Binary>,
		                  "Unsupported type for FuzzBinary:getSegment()");

		    if constexpr (std::is_base_of<MachO, std::remove_pointer_t<Binary>>)
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

		explicit Harness(const char *binary);
		explicit Harness(const char *binary, const char *mapFile);

		~Harness();

		struct FuzzBinary* getFuzzBinary() { return fuzzBinary; }

		template<typename Sym>
		Array<Sym>* getSymbols() requires requires (Sym sym) {
			{ sym->getName() };
			{ sym->getAddress() };
		}

		template<typename T>
		T getBinary() requires BinaryFormat<T> && PointerToClassType<T>()
		{
		    static_assert(BinaryFormat<T>,
		                  "Unsupported type for Module::getBinary()");

		    if constexpr (std::is_base_of<MachO, std::remove_pointer_t<Binary>>)
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

		template <typename CpuType> requires ScalarType<T>
		char* getMachOFromFatHeader(char *file_data);

		template<typename Binary> requires BinaryFormat<Binary>
		bool mapSegments(char *file_data, char *mapFile);

		template<typename Binary> requires BinaryFormat<Binary>
		bool unmapSegments();

		template<typename Binary> requires BinaryFormat<Binary>
		void getMappingInfo(char *file_data, size_t *size, uintptr_t *load_addr);

		void updateSegmentLoadCommandsForNewLoadAddress(char *file_data, uintptr_t newLoadAddress, uintptr_t oldLoadAddress);
		void updateSymbolTableForMappedMachO(char *file_data, uintptr_t newLoadAddress, uintptr_t oldLoadAddress);

		template<typename Binary = RawBinary> requires BinaryFormat<Binary> && !MachOFormat<Binary>
		void loadBinary(const char *path, const char *mapFile);

		void loadMachO(const char *path);

		void loadKernel(const char *path, off_t slide);
		void loadKernelExtension(const char *path);
		void loadKernelMachO(const char *kernelPath, uintptr_t *loadAddress, size_t *loadSize, uintptr_t *oldLoadAddress);

		template<typename Binary = RawBinary> requires BinaryFormat<Binary>
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