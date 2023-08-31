#ifndef __FUZZER_HPP_
#define __FUZZER_HPP_

namespace Fuzzer
{
	class Module;

	struct RawBinary
	{
		struct SegmentRaw
		{
			uint64_t address;

			size_t size;

			int prot;

			public:
				
				uint64_t getAddress() { return address; }

				size_t getSize() { return size; }

				int getProt() { return prot; }
		};

		struct SymbolRaw
		{
			const char *name;

			uint64_t address;

			int type;

			public:

				const char* getName() { return name; }

				uint64_t getAddress() { return address; }

				int getType() { return type; }
		};

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

		void loadBinary(const char *path, const char *symbolsFile);
		void loadKernel(const char *path, uintptr_t base, off_t slide);
		void loadMachO(const char *path);

		void populateSymbols(const char *symbolsFile);

		private:
			struct FuzzBinary *fuzzBinary;

			Loader *loader;
	};
};

#endif