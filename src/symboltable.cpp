//Andrew Guttman
//asguttma
//Xiaoli Tang
//xtang2

#include <string.h>
#include "auxlib.h"
#include "symboltable.h"

extern FILE *symfile;

symbol* new_symbol (attr_bitset attributes, size_t filenr,
  size_t linenr,size_t offset, size_t block_nr){
    symbol* sym = new symbol();
    sym->attributes = attributes;
    sym->filenr = filenr;
    sym->linenr = linenr;
    sym->offset = offset;
    sym->block_nr = block_nr;
    return sym;
}

void printAttr(symbol* value){
  if (value->attributes[ATTR_void] == true)
    fprintf(symfile," void");
  if (value->attributes[ATTR_bool] == true)
    fprintf(symfile," bool");
  if (value->attributes[ATTR_char] == true)
    fprintf(symfile," char");
  if (value->attributes[ATTR_int] == true)
    fprintf(symfile," int");
  if (value->attributes[ATTR_null] == true)
    fprintf(symfile," null");
  if (value->attributes[ATTR_string] == true)
    fprintf(symfile," string");
  if (value->attributes[ATTR_struct] == true){
    fprintf(symfile," struct");
    if (value->type != NULL)
    fprintf(symfile," \"%s\"",value->type->c_str());
  }
  if (value->attributes[ATTR_array] == true)
    fprintf(symfile," array");
  if (value->attributes[ATTR_function] == true)
    fprintf(symfile," function");
  if (value->attributes[ATTR_variable] == true)
    fprintf(symfile," variable");
  if (value->attributes[ATTR_field] == true)
    fprintf(symfile," field");
  if (value->attributes[ATTR_typeid] == true){}
    //fprintf(symfile," \"%s\"",value->type->c_str());
  if (value->attributes[ATTR_param] == true)
    fprintf(symfile," param");
  if (value->attributes[ATTR_lval] == true)
    fprintf(symfile," lval");
  if (value->attributes[ATTR_const] == true)
    fprintf(symfile," const");
  if (value->attributes[ATTR_vreg] == true)
    fprintf(symfile," vreg");
  if (value->attributes[ATTR_vaddr] == true)
    fprintf(symfile," vaddr");
  fprintf(symfile,"\n");
}

void printSymbol(const string* key, symbol* value,
  size_t indent){
  for (size_t i = indent-1; i>0; i--){
    fprintf(symfile, "  ");
  }
  fprintf(symfile, "%s (%lu.%lu.%lu) {%lu}", key->c_str(),
    value->filenr, value->linenr, value->offset, value->block_nr);
  printAttr(value);
}

void printField(const string* key, symbol* value,
  size_t indent, string structName){
  for (size_t i = indent-1; i>0; i--){
    fprintf(symfile, "  ");
  }
  fprintf(symfile, "%s (%lu.%lu.%lu) {%s}", key->c_str(),
    value->filenr, value->linenr, value->offset, structName.c_str());
  printAttr(value);
}

void printStruct(const string* key, symbol* value){
  fprintf(symfile,"%s (%lu.%lu.%lu) {%lu}", key->c_str(),
    value->filenr, value->linenr, value->offset, value->block_nr);
  printAttr(value);
}

void println(){
  fprintf(symfile,"\n");
}
