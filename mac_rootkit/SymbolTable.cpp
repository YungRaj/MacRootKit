#include "SymbolTable.hpp"

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