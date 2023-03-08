#include "SymbolTable.hpp"

#include <cxxabi.h>
#include <dlfcn.h>

#include <memory>

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

SymbolTable::SymbolTable()
{

}

SymbolTable::SymbolTable(struct nlist_64 *symtab, uint32_t nsyms, char *strtab, size_t strsize)
{
	this->symtab = symtab;
	this->nsyms = nsyms;
	this->strtab = strtab;
	this->strsize = strsize;
}

Symbol* SymbolTable::getSymbolByName(char *symname)
{
	for(int32_t i = 0; i < symbolTable.getSize(); i++)
	{
		Symbol *symbol = symbolTable.get(i);

		if(strcmp(symbol->getName(), symname) == 0)
		{
			return symbol;
		}
	}

	return NULL;
}

Symbol* SymbolTable::getSymbolByAddress(mach_vm_address_t address)
{
	for(int32_t i = 0; i < symbolTable.getSize(); i++)
	{
		Symbol *symbol = symbolTable.get(i);

		if(symbol->getAddress() == address)
		{
			return symbol;
		}
	}

	return NULL;
}

Symbol* SymbolTable::getSymbolByOffset(off_t offset)
{
	for(int32_t i = 0; i < symbolTable.getSize(); i++)
	{
		Symbol *symbol = symbolTable.get(i);

		if(symbol->getOffset() == offset)
		{
			return symbol;
		}
	}

	return NULL;
}