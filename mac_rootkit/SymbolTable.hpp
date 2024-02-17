/*
 * Copyright (c) YungRaj
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <sys/types.h>

#include "Symbol.hpp"

class Symbol;
class MachO;

class SymbolTable
{
	public:
		explicit SymbolTable() { }

		explicit SymbolTable(struct nlist_64 *symtab,
							 uint32_t nsyms,
							 char *strtab,
							 size_t strsize)
		    : symtab(symtab),
		      nsyms(nsyms),
		      strtab(strtab),
		      strsize(strsize)
		{

		}

		std::vector<Symbol*>& getAllSymbols() { return symbolTable; }

		bool containsSymbolNamed(char *name) { return this->getSymbolByName(name) != NULL; }

		bool containsSymbolWithAddress(mach_vm_address_t address) { return this->getSymbolByAddress(address) != NULL; }

		bool containsSymbolWithOffset(off_t offset) { return this->getSymbolByOffset(offset) != NULL; }

		Symbol* getSymbolByName(char *name);

		Symbol* getSymbolByAddress(mach_vm_address_t address);

		Symbol* getSymbolByOffset(off_t offset);

		void addSymbol(Symbol *symbol) { symbolTable.push_back(symbol); }

		void removeSymbol(Symbol *symbol) { symbolTable.erase(std::remove(symbolTable.begin(), symbolTable.end(), symbol), symbolTable.end()); }

		void replaceSymbol(Symbol *symbol);

	private:
		std::vector<Symbol*> symbolTable;

		struct nlist_64 *symtab;
		
		uint32_t nsyms;

		char *strtab;

		size_t strsize;
};