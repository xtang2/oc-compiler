//Andrew Guttman
//asguttma
//Xiaoli Tang
//xtang2

#ifndef __SYMTABLE_H__
#define __SYMTABLE_H__

#include <bitset>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>


// Instead of writing "std::string", we can now just write "string"
using namespace std;

// The enum for all the possible flags
//   ATTR_bitset_size is not actually used
//   but its value is equal to the number of attributes
enum { ATTR_void, ATTR_bool, ATTR_char, ATTR_int, ATTR_null,
ATTR_string, ATTR_struct, ATTR_array, ATTR_function,
ATTR_variable, ATTR_field, ATTR_typeid, ATTR_param, ATTR_lval,
ATTR_const, ATTR_vreg, ATTR_vaddr, ATTR_bitset_size };

// Create a shorthand notation for the bitset
using attr_bitset = bitset<ATTR_bitset_size>;

// Forward declaration of the symbol struct
struct symbol;


using symbol_entry = pair<const string*, symbol*>;

using symbol_table = unordered_map<const string*, symbol*>;

using struct_table = unordered_map<const string*, symbol*>;
// The actual definition of the symbol struct
struct symbol {
  attr_bitset attributes;
  symbol_table* fields;
  size_t filenr, linenr, offset;
  size_t block_nr;
  vector<symbol*>* parameters;
  const string* type;
  size_t declfilenr, decllinenr, decloffset;
};

symbol* new_symbol (attr_bitset attributes, size_t filenr,
   size_t linenr, size_t offset, size_t block_nr);

void printSymbol(const string* key, symbol* value,
   size_t indent);

void printField(const string* key, symbol* value,
   size_t inden, string structName);

void printStruct(const string* key, symbol* value);

void println();

//void addSymbol(symbol_table* symtbl, string* key, symbol* sym);

  
//string lookup(symbol_table symtable, string keys);
    

#endif
