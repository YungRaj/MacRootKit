#ifndef __FUZZER_HPP_
#define __FUZZER_HPP_

#include <type_traits>

extern "C"
{
	#include <stdio.h>
};

#include "BinaryFormat.hpp"

class MachO;
class KernelMachO;
class KextMachO;

class PE;
class TE;
class ELF;

extern "C"
{
	extern char* cxx_demangle(char *mangled);
	extern char* swift_demangle(char *mangled);
}

namespace Fuzzer
{
	class Loader;
	class Module;

	struct RawBinary : Binary::BinaryFormat
	{
		struct SegmentRaw
		{
			public:
				explicit SegmentRaw(const char *name, uintptr_t address, size_t size, int prot, int idx) : name(name), address(address), size(size), prot(prot), idx(idx) { }

				constexpr const char* getName() const { return name; }

				template<typename T>
				constexpr T operator[](uint64_t index) const { return reinterpret_cast<T>((uint8_t*) address + index); }

				constexpr uintptr_t getAddress() const { return address; }

				template<typename T>
				constexpr T getAddressAs() const
				{
					return reinterpret_cast<T>(address);
				}

				constexpr size_t getSize() const { return size; }

				constexpr int getProt() const { return prot; }

				constexpr int getIndex() const { return idx; }

			private:
				char *name;

				uintptr_t address;

				size_t size;

				int prot;
				int idx;

		};

		struct SymbolRaw
		{
			enum LanguageType
			{
				LANG_TYPE_C,
				LANG_TYPE_CXX,
				LANG_TYPE_CXX,
				LANG_TYPE_OBJC,
				LANG_TYPE_SWIFT,
				LANG_TYPE_RUST
			};

			template<LanguageType LangType>
			concept ManglableLang = LangType == LANG_TYPE_CXX || LangType == LANG_TYPE_SWIFT;

			public:
				explicit SymbolRaw(const char *name, uintptr_t address, int type) : name(name), address(address), type(type) { }

				constexpr const char* getName() const { return name; }

				template<LanguageType LangType> requires ManglableLang<LangType>
				char* getDemangledName()
				{
					if constexpr (LangType == LANG_TYPE_SWIFT)
					{
						char *_swift_demangle = swift_demangle(getName());

						if(_swift_demangle)
							return _swift_demangle;
					}

					if constexpr (LangType == LANG_TYPE_CXX)
					{
						char *_cxx_demangle = cxx_demangle(getName());

						if(_cxx_demangle)
							return _cxx_demangle;
					}

					char *empty = new char[1];

					*empty = '\0';

					return empty;
				}

				constexpr uintptr_t getAddress() const { return address; }

				constexpr int getType() const { return type; }

				constexpr bool isNamed() { return !name || strcmp(name, "") == 0; }

				constexpr bool isUndefined() const { return false; }

				constexpr bool isExternal() const { return false; }

			private:
				const char *name;

				uintptr_t address;

				int type;
		};

		public:

			explicit RawBinary(const char *path, const char *mapFile);

			template<typename T>
			constexpr T operator[](uint64_t index) const { return reinterpret_cast<T>((uint8_t*) base + index); }

			constexpr uintptr_t getBase() const { return base; }

			template<typename T>
			constexpr T getBaseAs() const
			{
				return reinterpret_cast<T>(base);
			}

			constexpr char* getMapFile() const { return mapFile; }

			constexpr Array<SymbolRaw*>* getAllSymbols() { return &symbols; }

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

			void populateSymbols();
			void populateSegments();

		private:
			uintptr_t base;

			char *mapFile;

			Array<SymbolRaw*> symbols;
			Array<SegmentRaw*> segments;
	};

	template <typename T>
	concept ClassType = std::is_class_v<T>;

	template<typename T>
	concept VoidPointerType = std::is_same_v<std::remove_pointer_t<T>, void>;

	template <typename T>
	concept PointerToClassType = (std::is_pointer_v<T> && std::is_class_v<std::remove_pointer_t<T>>) || VoidPointerType<T>;

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
		concept MachOFormat = std::is_base_of_v<MachO, std::remove_pointer_t<T>> || std::is_same_v<T, MachO*>;

		template<typename T>
		concept ELFFormat = std::is_same_v<T, ELF*>;

		template<typename T>
		concept PEFormat = std::is_same_v<T, PE*>;

		template<typename T>
		concept TEFormat = std::is_same_v<T, TE*>;

		template<typename T>
		concept RawBinaryFormat = std::is_same_v<T, RawBinary*>;

		template<typename T>
		concept BinaryFormat = MachOFormat<T> || ELFFormat<T> || PEFormat<T> || TEFormat<T> || RawBinaryFormat<T> || VoidPointerType<T>;

		static_assert(MachOFormat<KernelMachO*>);
		static_assert(MachOFormat<KextMachO*>);

		static_assert(RawBinaryFormat<RawBinary*>);

		template <typename T>
		using GetSymbolReturnType = decltype(std::declval<T>()->getSymbol(nullptr));

		template <typename T>
		using GetSegmentReturnType = decltype(std::declval<T>()->getSegment(nullptr));

		static_assert(std::is_same_v<GetSymbolReturnType<MachO*>, Symbol*>);
		static_assert(std::is_same_v<GetSymbolReturnType<RawBinary*>, SymbolRaw*>);

		template <PointerToClassType T> requires BinaryFormat<T>
		union Binary
		{
			/* We know the binary is a any of the below BinaryFormats */
			T bin;

			/* Support MachO */
			MachO *macho;

			/* Support Linux Binary fuzzing on ELFs */
			ELF *elf;

			/* Support EFI fuzzing on PE32/TE */
			PE *portableExecutable;

			TE *terseExecutable;

			/* Support Raw Binary */
			RawBinary *raw;

			/* Support void* to use type punning */
			void* binPtr;

			/* This union constrains the Binary Format types */
			/* The following binary formats are supported but may not be implemented */

			Binary() : binPtr(nullptr) {}

			Binary(T ptr) : bin(ptr) {}

			template <typename U> requires BinaryFormat<U>
			Binary(const Binary<U>& other)
			{
				static_assert(std::is_convertible_v<U, T>, "Incompatible BinaryFormat types for type punning");
				
				binPtr = static_cast<T>(static_cast<U>(other.binPtr));
			}
		};

		template<PointerToClassType T> requires BinaryFormat<T>
		constexpr Binary<T> MakeBinary(T ptr)
		{
		    return Binary<T>(ptr);
		}

		static_assert(std::is_same_v<GetSegmentReturnType<MachO*>, Segment*>);
		static_assert(std::is_same_v<GetSegmentReturnType<RawBinary*>, SegmentRaw*>);

		static_assert(BinaryFormat<decltype(binary.macho)> &&
					  BinaryFormat<decltype(binary.elf)>&&
					  BinaryFormat<decltype(binary.portableExecutable)> &&
					  BinaryFormat<decltype(binary.terseExecutable)> &&
					  BinaryFormat<decltype(binary.raw)>,
					  "All types in the union must satisfy the BinaryFormat concept");

		using AnyBinary = Binary<void*>;

		AnyBinary binary;

		template<typename T> requires CastableType<T>
		constexpr T operator[](uint64_t index) const { return reinterpret_cast<T>((uint8_t*) base + index); }
		
		constexpr const char* getPath() const { return path; }

		template<typename T> requires IntegralOrPointerType<T>
		constexpr T getBase() const
		{
			return reinterpret_cast<T>(base);
		}

		template<typename T> requires IntegralOrPointerType<T>
		constexpr T getOriginalBase() const
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

		    if constexpr (std::is_base_of_v<MachO, std::remove_pointer_t<Binary>>)
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

		    if constexpr (std::is_base_of_v<MachO, std::remove_pointer_t<Binary>>)
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

		constexpr struct FuzzBinary* getFuzzBinary() const { return fuzzBinary; }

		template<typename Sym>
		Array<Sym>* getSymbols() requires requires (Sym sym) {
			{ sym->getName() };
			{ sym->getAddress() };
		}

		template<typename T>
		T getBinary() requires BinaryFormat<T> && PointerToClassType<T>()
		{
		    static_assert(BinaryFormat<T>,
		                  "Unsupported type for Harness::getBinary()");

		    if constexpr (std::is_base_of_v<MachO, std::remove_pointer_t<Binary>>)
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

		constexpr Fuzzer::Loader* getLoader() const { return loader; }

		constexpr char* getMapFile() const { return mapFile; }

		template <typename CpuType> requires ScalarType<T>
		char* getMachOFromFatHeader(char *file_data);

		template<typename Binary, typename Seg> requires BinaryFormat<Binary>
		bool mapSegments(char *file_data, char *mapFile);

		template<typename Binary, typename Seg> requires BinaryFormat<Binary>
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

		template<typename Func, typename... Args, typename Binary, typename Sym> requires requires (Binary bin, Sym sym)
		{
			{ sym->getName() };
			{ sym->getAddress() };
			{ std::is_same_v<GetSymbolReturnType<Binary>, Sym> };
		};
		std::invoke_result_t<Func, Args...> execute(const char *name, Func func, Args... args);

		private:
			xnu::Kernel *kernel;

			xnu::KDKInfo *kdk;

			struct FuzzBinary *fuzzBinary;

			Fuzzer::Loader *loader;

			const char *mapFile;
	};
};

#endif