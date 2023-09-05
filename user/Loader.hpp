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
			explicit Module(const char *path) : path(path) { }

			explicit Module(const char *path, uintptr_t base, off_t slide) : path(path), base(base), slide(slide) { }

			~Module();

			const char* getName() { return name; }

			const char* getPath() { return path; }

			uintptr_t getEntryPoint();

			template<typename Sym>
			Array<Sym>* getSymbols() requires requires (Sym sym) {
				{ sym->getName() };
				{ sym->getAddress() };
			}

			template<typename Sym>
			Array<Sym>* getUndefinedSymbols() requires requires (Sym sym) {
				{ sym->getName() };
				{ sym->getAddress() };
				{ sym->isUndefined() };
			}

			template<typename Sym>
			Array<Sym>* getExternalSymbols() requires requires (Sym sym) {
				{ sym->getName() };
				{ sym->getAddress() };
				{ sym->isExternal(); }
			}

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
		        { seg->getAddress() };
		    }

		    template<typename Sect>
		    void mapSection(Sect section) requires requires(Sect sect) {
		        { sect->getAddress() };
		        { sect->getSize() };
		    }

		    template<typename Seg>
		    void modifySegment(Seg segment) requires requires(Seg seg) {
		        { seg->getAddress() };
		    }

		    template<typename Sect>
		    void modifySection(Sect section) requires requires(Sect sect) {
		        { sect->getAddress() };
		        { sect->getSize() };
		    }

		private:
			const char *name;
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
			explicit Loader(Fuzzer::Harness *harness, struct FuzzBinary *binary);

			~Loader();

			Architecture* getArchitecture() { return arch; }

			Fuzzer::Module* getModule(char *name)
			{
				for(int i = 0; i < modules.getSize(); i++)
				{
					Fuzzer::Module *module = module.get(i);

					if(strcmp(module->getName(), name) == 0)
					{
						return module;
					}
				}

				return NULL;
			}

			void loadModuleFromKext(const char *kextPath);

			void loadKextMachO(const char *kextPath, uintptr_t *loadAddress, size_t *loadSize, uintptr_t *oldLoadAddress);

			void linkSymbols(Module *module);
			void linkSymbol(Module *module, Symbol *symbol);

			void stubFunction(Module *module, Symbol *symbol, uintptr_t stub);

			void* allocateModuleMemory(uintptr_t addr, size_t sz, int prot);

		private:
			Fuzzer::Harness *harness;

			Architecture *arch;

			struct FuzzBinary *binary;

			Array<Module*> modules;
	};
};

#endif