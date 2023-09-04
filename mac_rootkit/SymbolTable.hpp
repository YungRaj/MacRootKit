#ifndef __SYMBOL_TABLE_HPP_
#define __SYMBOL_TABLE_HPP_

#include <sys/types.h>

#include "Symbol.hpp"
#include "MachO.hpp"

class Symbol;
class MachO;

class SymbolTable
{
	public:
		explicit SymbolTable();

		SymbolTable(struct nlist_64 *symtab, uint32_t nsyms, char *strtab, size_t strsize);

		std::Array<Symbol*> getAllSymbols() { return symbolTable; }

		bool containsSymbolNamed(char *name) { return this->getSymbolByName(name) != NULL; }

		bool containsSymbolWithAddress(mach_vm_address_t address) { return this->getSymbolByAddress(address) != NULL; }

		bool containsSymbolWithOffset(off_t offset) { return this->getSymbolByOffset(offset) != NULL; }

		Symbol* getSymbolByName(char *name);

		Symbol* getSymbolByAddress(mach_vm_address_t address);

		Symbol* getSymbolByOffset(off_t offset);

		void addSymbol(Symbol *symbol) { symbolTable.add(symbol); }

		void removeSymbol(Symbol *symbol) { symbolTable.remove(symbol); }

		void replaceSymbol(Symbol *symbol);

	private:
		std::Array<Symbol*> symbolTable;

		struct nlist_64 *symtab;
		
		uint32_t nsyms;

		char *strtab;

		size_t strsize;
};

#endif