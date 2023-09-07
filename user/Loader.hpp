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
			explicit Module(const char *path, struct FuzzBinary *binary) 
				: path(path), moduleBinary(binary), mainBinary() { }

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

			template<typename T>
			T operator[](uint64_t index) requires CastableType<T> { return reinterpret_cast<T>((uint8_t*) base + index); }

			template<typename T> requires IntegralOrPointerType<T>
			T getBase()
			{
				return reinterpret_cast<T>(base);
			}

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

			Fuzzer::Harness* getHarness() { return harness; }

			struct FuzzBinary* getFuzzBinary() { return binary; }

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

			template<typename Sym, typename Binary>
			void linkSymbols(Module *module) requires requires (Sym sym) {
				{ sym->getName() };
				{ sym->getAddress() };
				{ std::is_same_v<GetSymbolReturnType<Binary>, Sym> };
			};

			template<typename Sym, typename Binary>
			void linkSymbol(Module *module, Sym sym) requires requires (Sym sym){
				{ sym->getName() };
				{ sym->getAddress() };
				{ std::is_same_v<GetSymbolReturnType<Binary>, Sym> };
			};

			template<typename Sym, typename Binary>
			void stubFunction(Module *module, Sym sym, uintptr_t stub) requires requires (Sym sym) {
				{ sym->getName() };
				{ sym->getAddress() };
				{ std::is_same_v<GetSymbolReturnType<Binary>, Sym> };
			};

			void* allocateModuleMemory(uintptr_t addr, size_t sz, int prot);

		private:
			Fuzzer::Harness *harness;

			Architecture *arch;

			struct FuzzBinary *binary;

			Array<Module*> modules;
	};
};

#endif