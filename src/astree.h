//Andrew Guttman
//asguttma
//Xiaoli Tang
//xtang2

#ifndef __ASTREE_H__
#define __ASTREE_H__

#include <string>
#include <vector>
#include <bitset>
using namespace std;


#include "auxlib.h"
#include "symboltable.h"

struct astree {
   int symbol;               // token code
   size_t filenr;            // index into filename stack
   size_t linenr;            // line number from source code
   size_t offset;            // offset of token with current line
   const string* lexinfo;    // pointer to lexical information
   vector<astree*> children; // children of this n-way node
   attr_bitset attributes;
   size_t blocknr;
   const string* type;
   size_t declfilenr, decllinenr, decloffset;
   int declblocknr;
   astree* next;
   int reg;
};
symbol* new_symbol (attr_bitset attributes, size_t filenr,
  size_t linenr, size_t offset, size_t block_nr);

astree* new_astree (int symbol, int filenr, int linenr,
                    int offset, const char* lexinfo);
astree* adopt1 (astree* root, astree* child);
astree* adopt2 (astree* root, astree* left, astree* right);
astree* adopt1sym (astree* root, astree* child, int symbol);
astree* changesym (astree* node, int symbol);
void dump_astree (FILE* outfile, astree* root);
void yyprint (FILE* outfile, unsigned short toknum,
              astree* yyvaluep);
void free_ast (astree* tree);
void free_ast2 (astree* tree1, astree* tree2);

void visitTree (astree* root);
void pushTable ();
void processTree(astree* root);
void setNext(astree* root);

RCSH("$Id: astree.h,v 1.3 2014-12-12 22:45:37-08 - - $")
#endif
