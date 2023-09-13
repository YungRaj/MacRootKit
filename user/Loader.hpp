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
			explicit Module(Fuzzer::Loader *loader, const char *path, struct FuzzBinary *binary) 
				: loader(loader), path(path), moduleBinary(loader->getFuzzBinary()), mainBinary() { }

			explicit Module(const char *path, uintptr_t base, off_t slide) : path(path), base(base), slide(slide) { }

			~Module();

			constexpr Fuzzer::Loader* getLoader() const { return loader; }

			constexpr const char* getName() const { return name; }

			constexpr const char* getPath() const { return path; }

			uintptr_t getEntryPoint();

			template<typename Binary, typename Sym> requires BinaryFormat<Binary>
			constexpr Array<Sym>* getSymbols() requires requires (Sym sym) {
				{ sym->getName() };
				{ sym->getAddress() };
				{ bin->getAllSymbols() } -> std::same_as<Array<Sym>*>;
			}
			{
				return this->getBinary<Binary>()->getAllSymbols();
			}

			template<typename Binary, typename Sym> requires BinaryFormat<Binary>
			constexpr Array<Sym>* getUndefinedSymbols() requires requires (Sym sym) {
				{ sym->getName() };
				{ sym->getAddress() };
				{ sym->isUndefined() };
				{ bin->getAllSymbols() } -> std::same_as<Array<Sym>*>;
			}
			{
				Binary bin = this->getBinary<Binary>();

				Array<Sym> *syms = new Array<Sym>();
				Array<Sym> *allsyms = bin->getAllSymbols();

				for(int i = 0; i < allsyms->getSize(); i++)
				{
					Sym sym = allsyms->get(i);

					if(sym->isUndefined())
						syms->add(sym);
				}

				return syms;
			}

			template<typename Binary, typename Sym> requires BinaryFormat<Binary>
			constexpr Array<Sym>* getExternalSymbols() requires requires (Sym sym) {
				{ sym->getName() };
				{ sym->getAddress() };
				{ sym->isExternal() }
				{ bin->getAllSymbols() } -> std::same_as<Array<Sym>*>;
			}
			{
				Binary bin = this->getBinary<Binary>();

				Array<Sym> *syms = new Array<Sym>();
				Array<Sym> *allsyms = bin->getAllSymbols();

				for(int i = 0; i < allsyms->getSize(); i++)
				{
					Sym sym = allsyms->get(i);

					if(sym->isExternal())
						syms->add(sym);
				}

				return syms;
			}

			constexpr struct FuzzBinary* getMainBinary() const { return mainBinary; }

			constexpr struct FuzzBinary* getModuleBinary() const { return moduleBinary; }

			template<typename T>
			T getBinary() requires BinaryFormat<T> && PointerToClassType<T>
			{
			    static_assert(BinaryFormat<T>,
			                  "Unsupported type for Module::getBinary()");

			    if constexpr (std::is_base_of_v<MachO, std::remove_pointer_t<T>>)
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

			template<typename T> requires CastableType<T>
			constexpr T operator[](uint64_t index) const { return reinterpret_cast<T>((uint8_t*) base + index); }

			template<typename T> requires IntegralOrPointerType<T>
			constexpr T getBase() const
			{
				return reinterpret_cast<T>(base);
			}

			constexpr size_t getSize() const { return size; }

			constexpr off_t getSlide() const { return slide; }

		    void load();

		    template<typename Seg>
		    void mapSegment(Seg segment) requires requires(Seg seg) {
		        { seg->getAddress() };
		    }

		    template<typename Seg>
		    void modifySegment(Seg segment) requires requires(Seg seg) {
		        { seg->getAddress() };
		    }

		private:
			Fuzzer::Loader *loader;

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

			constexpr Fuzzer::Harness* getHarness() const { return harness; }

			constexpr struct FuzzBinary* getFuzzBinary() const { return binary; }

			constexpr inline Architecture* getArchitecture() const { return arch; }

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

			template<typename Binary> requires BinaryFormat<Binary>
			void loadModule(Module *module);

			void loadModuleFromKext(const char *kextPath);

			void loadKextMachO(const char *kextPath, uintptr_t *loadAddress, size_t *loadSize, uintptr_t *oldLoadAddress);

			template<typename Sym, typename Binary> requires BinaryFormat<Binary>
			void linkSymbols(Module *module) requires requires (Sym sym) {
				{ sym->getName() };
				{ sym->getAddress() };
				{ std::is_same_v<GetSymbolReturnType<Binary>, Sym> };
			};

			template<typename Sym, typename Binary> requires BinaryFormat<Binary>
			void linkSymbol(Module *module, Sym sym) requires requires (Sym sym){
				{ sym->getName() };
				{ sym->getAddress() };
				{ std::is_same_v<GetSymbolReturnType<Binary>, Sym> };
			};

			template<typename Sym, typename Binary> requires BinaryFormat<Binary>
			void stubFunction(Module *module, Sym sym, uintptr_t stub) requires requires (Sym sym) {
				{ sym->getName() };
				{ sym->getAddress() };
				{ std::is_same_v<GetSymbolReturnType<Binary>, Sym> };
			};

			template<typename Sym, typename Binary> requires BinaryFormat<Binary>
			void shimFunction(Module *module, Sym sym, uintptr_t stub) requires requires (Sym sym) {
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