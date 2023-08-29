#ifndef __LOADER_HPP_
#define __LOADER_HPP_

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

			struct FuzzBinary* getBinary() { return binary; }

			size_t getSize() { return size; }

			off_t getSlide() { return slide; }

			template<typename T>
			T getSymbol(const char *symname);

			void load();

			template<typename T>
			void mapSegment(T segment);

			template<typename T>
			void mapSection(T section);

		private:
			const char *path;

			struct FuzzBinary *binary;

			uintptr_t base;

			size_t size;

			off_t slide;
	};

	class Loader
	{
		public:
			explicit Loader(struct FuzzBinary *binary);

			~Loader();

			void* allocateSegmentMemory(uintptr_t addr, size_t sz, int prot);

			void linkSymbols(Module *module);
			void linkSymbol(Module *module, Symbol *symbol);

			void stubFunction(Module *module, Symbol *symbol, uintptr_t stub);

		private:
			struct FuzzBinary *binary;

			Array<Module*> modules;
	};
};

#endif