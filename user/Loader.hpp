#ifndef __LOADER_HPP_
#define __LOADER_HPP_

#include <type_traits>

#include "Fuzzer.hpp"
#include "Array.hpp"

class MachO;
class Symbol;

class Architecture;

namespace Fuzzer
{
	struct FuzzBinary;

	class Module
	{
		public:
			explicit Module(const char *path);

			explicit Module(const char *path, uintptr_t base, off_t slide);

			~Module();

			const char* getPath() { return path; }

			struct FuzzBinary* getMainBinary() { return mainBinary; }

			struct FuzzBinary* getModuleBinary() { return moduleBinary; }

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

			uintptr_t getBase() { return base; }

			size_t getSize() { return size; }

			off_t getSlide() { return slide; }

		    void load();

		    template<typename Seg>
		    void mapSegment(Seg segment) requires requires(Seg seg) {
		        { seg.getAddress() };
		    }

		    template<typename Sect>
		    void mapSection(Sect section) requires requires(Sect sect) {
		        { sect.getAddress() };
		        { sect.getSize() };
		    }

		private:
			const char *path;

			struct FuzzBinary *mainBinary;
			struct FuzzBinary *moduleBinary;

			uintptr_t base;

			size_t size;

			off_t slide;
	};

	class Loader
	{
		public:
			explicit Loader(struct FuzzBinary *binary);

			~Loader();

			void linkSymbols(Module *module);
			void linkSymbol(Module *module, Symbol *symbol);

			void stubFunction(Module *module, Symbol *symbol, uintptr_t stub);

			void* allocateSegmentMemory(uintptr_t addr, size_t sz, int prot);

		private:
			Architecture *arch;

			struct FuzzBinary *binary;

			Array<Module*> modules;
	};
};

#endif