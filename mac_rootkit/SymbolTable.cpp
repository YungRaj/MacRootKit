#include "SymbolTable.hpp"

#ifdef __USER__

#include <cxxabi.h>
#include <dlfcn.h>

#include <memory>

#endif

extern "C"
{
#ifdef __USER__

	char* cxx_demangle(char *mangled)
	{
		int status;

		char *ret = abi::__cxa_demangle(mangled, 0, 0, &status);  

		std::shared_ptr<char> retval;

		retval.reset( (char *)ret, [](char *mem) { if (mem) free((void*)mem); } );

		return static_cast<char*>(retval.get());
	}

	typedef char* (*_swift_demangle) (char *mangled, uint32_t length, uint8_t *output_buffer, uint32_t output_buffer_size, uint32_t flags);

	char* swift_demangle(char *mangled)
	{
		void *runtime_loader_default = dlopen(NULL, RTLD_NOW);

		if(runtime_loader_default)
		{
			void *sym = dlsym(runtime_loader_default, "swift_demangle");

			if(sym)
			{
				_swift_demangle f = reinterpret_cast<_swift_demangle>(sym);

				char *cString = f(mangled, strlen(mangled), NULL, 0, 0);
				
				if(cString)
				{
					dlclose(runtime_loader_default);

					return cString;
				}
			}

			dlclose(runtime_loader_default);
		}

		return NULL;
	}

#else

	char* cxx_demangle(char *mangled) { return NULL; }
	char* swift_demangle(char *mangled) { return NULL; }

#endif

}

Symbol* SymbolTable::getSymbolByName(char *symname)
{
	for(int32_t i = 0; i < symbolTable.size(); i++)
	{
		Symbol *symbol = symbolTable.at(i);

		if(strcmp(symbol->getName(), symname) == 0)
		{
			return symbol;
		}
	}

	return NULL;
}

Symbol* SymbolTable::getSymbolByAddress(mach_vm_address_t address)
{
	for(int32_t i = 0; i < symbolTable.size(); i++)
	{
		Symbol *symbol = symbolTable.at(i);

		if(symbol->getAddress() == address)
		{
			return symbol;
		}
	}

	return NULL;
}

Symbol* SymbolTable::getSymbolByOffset(off_t offset)
{
	for(int32_t i = 0; i < symbolTable.size(); i++)
	{
		Symbol *symbol = symbolTable.at(i);

		if(symbol->getOffset() == offset)
		{
			return symbol;
		}
	}

	return NULL;
}

void SymbolTable::replaceSymbol(Symbol *symbol)
{
	for(size_t i = symbolTable.size() - 1; i >= 0; i--)
	{
		Symbol *sym = symbolTable.at(i);

		if(strcmp(sym->getName(), symbol->getName()) == 0)
		{
			symbolTable.erase(std::remove(symbolTable.begin(), symbolTable.end(), sym), symbolTable.end());
		}
	}

	symbolTable.push_back(symbol);
}