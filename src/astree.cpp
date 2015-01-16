//Andrew Guttman
//asguttma
//Xiaoli Tang
//xtang2

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "astree.h"
#include "stringset.h"
#include "lyutils.h"


string empty = "";

vector<symbol_table*> symbol_stack;
vector<int> blockStack;
struct_table structTable;
int next_block=1;

bool madeBlock = false;
bool insertMade = false;
bool madeLine = false;

    void setNext(astree* node){
      for (size_t j = 0; j < node->children.size(); j++) {
                //printf("in loop\n");
                setNext(node->children[j]);
      }
      if (node->children.size() == 0) return;
      for (size_t i = 0; i < node->children.size()-1; i++){
        node->children[i]->next = node->children[i+1];
      }
    }

astree* new_astree (int symbol, int filenr, int linenr,
                    int offset, const char* lexinfo) {
   astree* tree = new astree();
   tree->symbol = symbol;
   tree->filenr = filenr;
   tree->linenr = linenr;
   tree->offset = offset;
   tree->blocknr = 0;
   
   if (strcmp(lexinfo, "") == 0) tree->lexinfo = &empty;
   else tree->lexinfo = intern_stringset (lexinfo);
   DEBUGF ('f', "astree %p->{%d:%d.%d: %s: \"%s\"}\n",
           tree, tree->filenr, tree->linenr, tree->offset,
           get_yytname (tree->symbol), tree->lexinfo->c_str());
   return tree;
}

astree* adopt1 (astree* root, astree* child) {
   root->children.push_back (child);
   DEBUGF ('a', "%p (%s) adopting %p (%s)\n",
           root, root->lexinfo->c_str(),
           child, child->lexinfo->c_str());
   return root;
}

astree* adopt2 (astree* root, astree* left, astree* right) {
   adopt1 (root, left);
   adopt1 (root, right);
   return root;
}

astree* adopt1sym (astree* root, astree* child, int symbol) {
   root = adopt1 (root, child);
   root->symbol = symbol;
   return root;
}

astree* changesym (astree* node, int symbol) {
  node->symbol = symbol;
  return node;
}

int is_base(astree* node){
 
 if(node->attributes[ATTR_void]){
 return ATTR_void;
 }
 
 if(node->attributes[ATTR_bool]){
 return ATTR_bool;
 }
 
 if(node->attributes[ATTR_char]){
 return ATTR_char;
 }
 
 if(node->attributes[ATTR_int]){
 return ATTR_int;
 }
 
 if(node->attributes[ATTR_string]){
 return ATTR_string;
 }
 
 if(node->attributes[ATTR_typeid]){
 return ATTR_typeid;
 }
  //printf("%s\n", node->lexinfo->c_str());
  if (node->symbol != '.')
  errprintf("not a valid basetype\n");
  return -1;
 }

void printAttr(FILE* outfile, astree* node){
 if (node->attributes[0]) fprintf(outfile," void");
 if (node->attributes[1]) fprintf(outfile," bool");
 if (node->attributes[2]) fprintf(outfile," char");
 if (node->attributes[3]) fprintf(outfile," int");
 if (node->attributes[4]) fprintf(outfile," null");
 if (node->attributes[5]) fprintf(outfile," string");
 if (node->attributes[6]) {
    fprintf(outfile," struct");
    if (node->type != NULL) fprintf(outfile,
      " \"%s\"",node->type->c_str());
  }
 if (node->attributes[7]) fprintf(outfile," array");
 if (node->attributes[8]) fprintf(outfile," function");
 if (node->attributes[9]) fprintf(outfile," variable");
 if (node->attributes[10]) fprintf(outfile," field");
 //if (node->attributes[11]) fprintf(outfile," typeid");
 if (node->attributes[12]) fprintf(outfile," param");
 if (node->attributes[13]) fprintf(outfile," lval");
 if (node->attributes[14]) fprintf(outfile," const");
 if (node->attributes[15]) fprintf(outfile," vreg");
 if (node->attributes[16]) fprintf(outfile," vaddr"); 
}


static void dump_node (FILE* outfile, astree* node) {
   const char* tname = get_yytname(node->symbol);
   if (strstr (tname, "TOK_") == tname) tname+=4;
   fprintf (outfile, "%s \"%s\" (%lu.%lu.%lu) {%lu}",
            tname, node->lexinfo->c_str(),
            node->filenr, node->linenr, node->offset, node->blocknr);
   printAttr(outfile,node);  
   if (node->symbol == TOK_IDENT){
    fprintf(outfile, "(%lu.%lu.%lu)",
      node->declfilenr, node->decllinenr, node->decloffset);
   }
   bool need_space = false;
   for (size_t child = 0; child < node->children.size();
        ++child) {
      if (need_space) fprintf (outfile, " ");
      need_space = true;
      //fprintf (outfile, "%p", node->children.at(child));
   }
   //fprintf (outfile, "]}");
}

static void dump_astree_rec (FILE* outfile, astree* root,
                             int depth) {
   if (root == NULL) return;
   for (int i = 0; i < depth; i++){
      fprintf(outfile, "|   ");
   }
   // fprintf (outfile, "%*s%s ", depth * 3, "",
   //          root->lexinfo->c_str());
   dump_node (outfile, root);
   fprintf (outfile, "\n");
   for (size_t child = 0; child < root->children.size();
        ++child) {
      dump_astree_rec (outfile, root->children[child],
                       depth + 1);
   }
}

void dump_astree (FILE* outfile, astree* root) {
   dump_astree_rec (outfile, root, 0);
   fflush (NULL);
}

void yyprint (FILE* outfile, unsigned short toknum,
              astree* yyvaluep) {
   if (is_defined_token (toknum)) {
      dump_node (outfile, yyvaluep);
   }else {
      fprintf (outfile, "%s(%d)\n",
               get_yytname (toknum), toknum);
   }
   fflush (NULL);
}

void free_ast (astree* root) {
   while (not root->children.empty()) {
      astree* child = root->children.back();
      root->children.pop_back();
      free_ast (child);
   }
   DEBUGF ('f', "free [%p]-> %d:%d.%d: %s: \"%s\")\n",
           root, root->filenr, root->linenr, root->offset,
           get_yytname (root->symbol), root->lexinfo->c_str());
   delete root;
}

void free_ast2 (astree* tree1, astree* tree2) {
   free_ast (tree1);
   free_ast (tree2);
}

void checkTable(symbol_table** table){
  if (*table == nullptr){
    *table = new symbol_table;
  }
}

void makeFuncTypeArray(astree* node){
  switch (node->children[0]->symbol){
    case TOK_INT:{node->children[1]->attributes[ATTR_int]=true;
      break;}
    case TOK_CHAR:{node->children[1]->attributes[ATTR_char]=true;
      break;}
    case TOK_BOOL:{node->children[1]->attributes[ATTR_bool]=true;
      break;}
    case TOK_VOID:{node->children[1]->attributes[ATTR_void]=true;
      break;}
    case TOK_STRING:{node->children[1]->attributes[ATTR_string]=true;
      break;}
    case TOK_TYPEID:{node->children[1]->attributes[ATTR_typeid]=true;
      node->children[1]->attributes[ATTR_struct]=true;
      node->children[1]->attributes[ATTR_struct] = true;
      //node->children[1]->type = node->children[0]->lexinfo;
      break;}
    default:{
      break;
    }
  }
}

void makeAttr(astree* node){
  switch (node->symbol){
    case TOK_INT:{node->attributes[ATTR_int]=true;
      break;}
    case TOK_CHAR:{node->attributes[ATTR_char]=true;
      break;}
    case TOK_BOOL:{node->attributes[ATTR_bool]=true;
      break;}
    case TOK_VOID:{node->attributes[ATTR_void]=true;
      break;}
    case TOK_STRING:{node->attributes[ATTR_string]=true;
      break;}
    case TOK_TYPEID:{node->attributes[ATTR_typeid]=true;
      node->attributes[ATTR_struct]=true;
      node->attributes[ATTR_typeid] = true;
      node->type = node->lexinfo;
      break;}
    default:{
      break;
    }
  }
}

void makeFuncType(astree* node){
  switch (node->symbol){
    case TOK_INT:{node->children[0]->attributes[ATTR_int]=true;
      break;}
    case TOK_CHAR:{node->children[0]->attributes[ATTR_char]=true;
      break;}
    case TOK_BOOL:{node->children[0]->attributes[ATTR_bool]=true;
      break;}
    case TOK_VOID:{node->children[0]->attributes[ATTR_void]=true;
      break;}
    case TOK_STRING:{node->children[0]->attributes[ATTR_string]=true;
      break;}
    case TOK_TYPEID:{node->children[0]->attributes[ATTR_typeid]=true;
      node->children[0]->attributes[ATTR_struct]=true;
      node->children[0]->attributes[ATTR_typeid] = true;
      node->children[0]->type = node->lexinfo;
      break;}
    default:{
      break;
    }
  }
}

void makeStructSym(astree* root){
  const string* key = root->children[0]->lexinfo;
  root->children[0]->attributes[ATTR_struct] = true;
  root->children[0]->attributes[ATTR_typeid] = true;
  root->children[0]->type=root->children[0]->lexinfo;
  symbol* value = new symbol();
  value->filenr = root->children[0]->filenr;
  value->linenr = root->children[0]->linenr;
  value->offset = root->children[0]->offset;
  value->attributes = root->children[0]->attributes;
  value->type=root->children[0]->type;
  value->block_nr = 0;
  value->fields = nullptr;
  structTable[key] = value;
  insertMade = true;
  printStruct(key, value);
  if (root->children.size()>1){
    //value->fields = new symbol_table;
    checkTable(&(value->fields));
    for (size_t i=1; i < root->children.size(); i++){
      if (root->children[i]->symbol==TOK_ARRAY){
        astree* curr = root->children[i]->children[1];
        const string* key2 = curr->lexinfo;
        curr->blocknr = 0;
        curr->attributes[ATTR_field]=true;
        curr->attributes[ATTR_array]=true;
        makeFuncTypeArray(root->children[i]);
        symbol* value2 = new_symbol
        (curr->attributes, curr->filenr,
          curr->linenr, curr->offset, curr->blocknr);
        value2->type = curr->type;
        //symbol* value2 = new symbol();
        //printf("field: %s\n", key->c_str());
        (*value->fields)[key2] = value2;
        insertMade = true;
        printField(key2, value2, 2,
          root->children[0]->lexinfo->c_str());
      }
      else{
        astree* curr = root->children[i]->children[0];
        const string* key2 = curr->lexinfo;
        curr->blocknr = 0;
        curr->attributes[ATTR_field]=true;
        makeFuncType(root->children[i]);
        symbol* value2 = new_symbol
        (curr->attributes, curr->filenr,
          curr->linenr, curr->offset, curr->blocknr);
        value2->type = curr->type;
        //symbol* value2 = new symbol();
        //printf("field: %s\n", key->c_str());
        (*value->fields)[key2] = value2;
        insertMade = true;
        printField(key2, value2, 2,
          root->children[0]->lexinfo->c_str());
      }
    }
  }
}

void setAttr(astree* node, symbol* value){
  if (node->symbol==TOK_INT)
    value->attributes[ATTR_int]=true;
  if (node->symbol==TOK_CHAR)
    value->attributes[ATTR_char]=true;
  if (node->symbol==TOK_BOOL)
    value->attributes[ATTR_bool]=true;
  if (node->symbol==TOK_STRING)
    value->attributes[ATTR_string]=true;
  if (node->symbol==TOK_TYPEID)
    value->attributes[ATTR_typeid]=true;
}

void funcSym(astree* root){
  symbol* value = new symbol();
  const string* key = nullptr;
  if (root->children[0]->symbol == TOK_ARRAY){
    makeFuncTypeArray(root->children[0]);
    key = root->children[0]->children[1]->lexinfo;
    root->children[0]->children[1]->attributes[ATTR_function]
     = true;
    root->children[0]->children[1]->attributes[ATTR_array] = true;
    value->filenr = root->children[0]->children[1]->filenr;
    value->linenr = root->children[0]->children[1]->linenr;
    value->offset = root->children[0]->children[1]->offset;
    value->attributes = root->children[0]->children[1]->attributes;
    value->type=root->children[0]->children[1]->type;
  }else{
    makeFuncType(root->children[0]);
    key = root->children[0]->children[0]->lexinfo;
    root->children[0]->children[0]->attributes[ATTR_function]
     = true;
    value->filenr = root->children[0]->children[0]->filenr;
    value->linenr = root->children[0]->children[0]->linenr;
    value->offset = root->children[0]->children[0]->offset;
    value->attributes = root->children[0]->children[0]->attributes;
    value->type=root->children[0]->children[0]->type;
  }

  value->block_nr = 0;
  value->parameters = new vector<symbol*>;
  for (size_t i = 0; i < root->children[1]->children.size(); i++){
    symbol* v = new symbol;
    setAttr(root->children[1]->children[i],v);
    value->parameters->push_back(v);
    root->children[1]->children[i]->children[0]
    ->attributes[ATTR_param]=true;
    if (root->children[1]->children[i]->symbol == TOK_ARRAY){
      root->children[1]->children[i]->children[1]
      ->attributes[ATTR_param]=true;
    }
  }

  (*symbol_stack[0])[key]= value;
   insertMade = true;
   printSymbol(key, value, 1);

  symbol_stack.push_back(nullptr);
  blockStack.push_back(next_block);
  next_block++;
  madeBlock = true;
}

void processNode(astree* root){
  switch(root->symbol){
    case TOK_DECLID:{
      root->declblocknr = -1;
      if (root->attributes[ATTR_function]==true)
        break;
      root->attributes[ATTR_variable] = true;
      root->attributes[ATTR_lval]=true;  
      root->blocknr = blockStack.back();
      symbol* value = new_symbol
      (root->attributes, root->filenr, root->linenr,
        root->offset, root->blocknr);
      value->type = root->type;
      const string* key = root->lexinfo;
      
     
      checkTable(&symbol_stack[symbol_stack.size()-1]);

      if ((*symbol_stack[symbol_stack.size()-1]).count(key)>0){
            fprintf(stderr, "%s ", key->c_str());
            errprintf("variable already declared in this scope, error\n"
             , get_execname());
      }else{
            //printf("not declared, inserting\n");
            printSymbol(key, value, blockStack.size());
            (*symbol_stack[symbol_stack.size()-1])[key]= value;
            insertMade = true;
      }
      break;
    }
    case TOK_IDENT:{
      bool declared = false;
      const string* key = root->lexinfo;
          //symbol_table currScope = table;
      root->blocknr = blockStack.back();
      //printf("stack size %d\n", symbol_stack.size());

      for (int i=symbol_stack.size()-1; i>=0; i--){
        if (symbol_stack[i] != nullptr) {
           if ((*symbol_stack[i]).count(key)>0){
             root->attributes = (*symbol_stack[i])[key]->attributes;
             root->declfilenr = (*symbol_stack[i])[key]->filenr;
             root->decllinenr = (*symbol_stack[i])[key]->linenr;
             root->decloffset = (*symbol_stack[i])[key]->offset;
             root->declblocknr = (*symbol_stack[i])[key]->block_nr;
             root->type = (*symbol_stack[i])[key]->type;
             //printf("checking depth %d\n", i);
             declared = true;
             break;
           }
        }
      }

      if (declared){
        //printf("properly declared\n");
            //already declared in scope
      }else{
        fprintf(stderr, "%s ", key->c_str());
        errprintf("variable not declared in viewable scope, error\n",
          get_execname());
            //error
      }
      break;
    }
    case TOK_FUNCTION:{
      root->blocknr = blockStack.back();
      funcSym(root);
      break;
    }
    case TOK_PROTOTYPE:{
      root->blocknr = blockStack.back();
     //printf("in proto\n");

      funcSym(root);
      break;
    }
    case TOK_STRUCT:{
      root->blocknr = blockStack.back();
      if (structTable.count(root->children[0]->lexinfo)>0){
        if (structTable[root->children[0]->lexinfo]->fields != nullptr){
         errprintf("struct already declared, error\n");
          break;
        }
      }
      makeStructSym(root);
      break;
    }
    case TOK_BLOCK:{
      root->blocknr = blockStack.back();
      //symbol_table table;
      if (madeBlock==false){
        symbol_stack.push_back(nullptr);
        blockStack.push_back(next_block);
        next_block++;
        //printf("push block\n");
      }else{
        madeBlock=false;
      }
      break;
    }
    case TOK_TYPEID:{
      root->blocknr = blockStack.back();
      if (root->children.size()==0)
        break;
      if (structTable.count(root->lexinfo) < 1){
        errprintf("struct not defined, error\n");
      }else{
        root->attributes 
         = (structTable[root->lexinfo]->attributes | root->attributes);
        root->type = root->lexinfo;
      }
      if (root->children.size()>0){
        for (size_t child=0; child < root->children.size(); ++child){
          root->children[child]->attributes[ATTR_struct] = true;
          root->children[child]->attributes[ATTR_typeid] = true;
          root->children[child]->type = root->lexinfo;
        }
      }
      break;
    }
    case TOK_FIELD:{
      root->blocknr = blockStack.back();
      root->attributes[ATTR_field]=true;
      break;
    }
    case TOK_INT:{
      root->blocknr = blockStack.back();
      if (root->children.size()==0)
        break;
      root->children[0]->attributes[ATTR_int]=true;
      break;
    }
    case TOK_CHAR:{
      root->blocknr = blockStack.back();
      if (root->children.size()==0)
        break;
      root->children[0]->attributes[ATTR_char]=true;
      break;
    }
    case TOK_STRING:{
      root->blocknr = blockStack.back();
      if (root->children.size()==0)
        break;
      root->children[0]->attributes[ATTR_string]=true;
      break;
    }
    case TOK_BOOL:{
      root->blocknr = blockStack.back();
      if (root->children.size()==0)
        break;
      root->children[0]->attributes[ATTR_bool]=true;
      break;
    }
    case TOK_VOID:{
      root->blocknr = blockStack.back();
      if (root->children.size()==0)
        break;
      root->children[0]->attributes[ATTR_void]=true;
      break;
    }
    case TOK_ARRAY:{
      root->blocknr = blockStack.back();
      makeAttr(root->children[0]);
      root->children[1]->attributes[ATTR_array]=true;
      root->children[1]->attributes[is_base(root->children[0])]=true;
      root->attributes[is_base(root->children[0])]=true;
      //root->children[1]->attributes[ATTR_typeid]=true;
      //root->children[0]->attributes[ATTR_typeid]=true;
      break;
    }
    case TOK_NEWARRAY:{
      root->blocknr = blockStack.back();
      root->children[0]->attributes[ATTR_typeid]=true;
      break;
    }
    case TOK_IF:{
      root->blocknr = blockStack.back();
      break;
    }
    case TOK_ELSE:{
      root->blocknr = blockStack.back();
      break;
    }
    case TOK_WHILE:{
      root->blocknr = blockStack.back();
      break;
    }
    case TOK_RETURN:{
      root->blocknr = blockStack.back();
      break;
    }
    case TOK_FALSE:{
      root->blocknr = blockStack.back();
      break;
    }
    case TOK_TRUE:{
      root->blocknr = blockStack.back();
      break;
    }
    case TOK_NULL:{
      root->blocknr = blockStack.back();
      break;
    }
    case TOK_NEW:{
      root->blocknr = blockStack.back();
      break;
    }
    case TOK_EQ:{
      root->blocknr = blockStack.back();
      break;
    }
    case TOK_NE:{
      root->blocknr = blockStack.back();
      break;
    }
    case TOK_LT:{
      root->blocknr = blockStack.back();
      break;
    }
    case TOK_LE:{
      root->blocknr = blockStack.back();
      break;
    }
    case TOK_GT:{
      root->blocknr = blockStack.back();
      break;
    }
    case TOK_GE:{
      root->blocknr = blockStack.back();
      break;
    }
    case TOK_INTCON:{
      root->blocknr = blockStack.back();
      break;
    }
    case TOK_CHARCON:{
      root->blocknr = blockStack.back();
      break;
    }
    case TOK_STRINGCON:{
      root->blocknr = blockStack.back();
      break;
    }
    case TOK_CALL:{
      root->blocknr = blockStack.back();
      break;
    }
    case TOK_IFELSE:{
      root->blocknr = blockStack.back();
      break;
    }
    case TOK_INITDECL:{
      root->blocknr = blockStack.back();
      break;
    }
    case TOK_POS:{
      root->blocknr = blockStack.back();
      break;
    }
    case TOK_NEG:{
      root->blocknr = blockStack.back();
      break;
    }
    case TOK_ORD:{
      root->blocknr = blockStack.back();
      break;
    }
    case TOK_CHR:{
      root->blocknr = blockStack.back();
      break;
    }
    case TOK_NEWSTRING:{
      root->blocknr = blockStack.back();
      break;
    }
    case TOK_INDEX:{
      root->blocknr = blockStack.back();
      break;
    }
    case TOK_RETURNVOID:{
      root->blocknr = blockStack.back();
      break;
    }
    case TOK_VARDECL:{
      root->blocknr = blockStack.back();
      break;
    }
    case TOK_PARAMLIST:{
      root->blocknr = blockStack.back();
      break;
    }
    case '(':{
      root->blocknr = blockStack.back();
      break;
    }
    case ')':{
      root->blocknr = blockStack.back();
      break;
    }
    case '[':{
      root->blocknr = blockStack.back();
      break;
    }
    case ']':{
      root->blocknr = blockStack.back();
      break;
    }
    case '{':{
      root->blocknr = blockStack.back();
      break;
    }
    case '}':{
      root->blocknr = blockStack.back();
      break;
    }
    case ';':{
      root->blocknr = blockStack.back();
      break;
    }
    case ',':{
      root->blocknr = blockStack.back();
      break;
    }

    case '.' :{
    root->blocknr = blockStack.back();
    //printf("%s\n", root->children[0]->type->c_str());
    if (structTable.count(root->children[0]->type) > 0){
      if ((structTable[root->children[0]->type]->fields->
        count(root->children[1]->lexinfo))>0){
        
        root->children[1]->attributes =
         (*structTable[root->children[0]->type]->fields)
         [root->children[1]->lexinfo]->attributes;
        //printf("valid\n");//valid
      }else{//printf("error in\n");
      //errprintf("type error\n");
      } //error
    }else{//printf("error out\n");
    //errprintf("type error\n");
    } //error
    break;
    }

    case '=':{
      root->blocknr = blockStack.back();
      break;
    }
    case '+':{
      root->blocknr = blockStack.back();
      break;
    }
    case '-':{
      root->blocknr = blockStack.back();
      break;
    }
    case '*':{
      root->blocknr = blockStack.back();
      break;
    }
    case '/':{
      root->blocknr = blockStack.back();
      break;
    }
    case '%':{
      root->blocknr = blockStack.back();
      break;
    }
    case '!':{
      root->blocknr = blockStack.back();
      break;
    }
    default:{
      break;
    }
  }
}

void dfs (astree* root){
   processNode(root);
   for (size_t child=0; child < root->children.size(); ++child){
     madeLine=false;
     dfs(root->children[child]);
     //processNode(root);
     //dfs(root->children[child]);
     //procressNode(root);
     if (root->symbol == TOK_BLOCK && child == root->children.size()-1){
        symbol_stack.pop_back();
        blockStack.pop_back();
        //printf("pop block\n");
        madeBlock=false;
     }
     if (root->symbol == TOK_PROTOTYPE && child
      == root->children.size()-1){
        symbol_stack.pop_back();
        blockStack.pop_back();
        //printf("pop proto\n");
        madeBlock=false;
     }
     if (root->symbol == TOK_PARAMLIST && child
      == root->children.size()-1){
        println();
        madeLine = true;
     }
     if (blockStack.size()==1 && insertMade && !madeLine ){
        println();
        insertMade = false;
     }
   }
}

void processTree(astree* root){
   symbol_table table;
   symbol_stack.push_back(&table);
   blockStack.push_back(0);
   dfs(root);
}

int is_primitive (astree* node) {   
  
  if(node->attributes[ATTR_int]){
  return ATTR_int;
  }
  
  if(node->attributes[ATTR_char]){
  return ATTR_char;
  }
  
  if(node->attributes[ATTR_bool]){ 
  return ATTR_bool;
  }
  //printf("%s\n", node->lexinfo->c_str());
  //errprintf("not a primitive type\n");
  return -1;
}


int is_ref (astree* node) {
 if(node->attributes[ATTR_null]){
 return ATTR_null;
 }
 
 if(node->attributes[ATTR_string]){
 return ATTR_string;
 }
 
 if(node->attributes[ATTR_struct]){
 return ATTR_struct;
 }
 
 if(node->attributes[ATTR_array]){
 return ATTR_array;
 }
  //printf("%s\n", node->lexinfo->c_str());
  //errprintf("not a reference type\n");
  return -1;
 
}         
         
int is_any (astree* node){
 if(is_primitive(node)>=0){
 return is_primitive(node);
 }
 if(is_ref(node)>=0){
 return is_ref(node);
 }
 //printf("%s\n", node->lexinfo->c_str());
 //errprintf("not a valid type\n");
 return -1;
 }

void check_oper(astree* root){
  if (root->children[0]->attributes[ATTR_array]||
    root->children[1]->attributes[ATTR_array]){
    errprintf("type error, adding arrays\n");
  }
 if(root->children[0]->attributes[ATTR_int]&&root->children[1]
    ->attributes[ATTR_int]){
  root->attributes[ATTR_int] = true;
  root->attributes[ATTR_vreg] = true;
  }
  else{
    }
}

void check_compar(astree* root){
//printf("attribute num: %d\n",is_primitive(root->children[0]));
//printf("attribute num1: %d\n",is_primitive(root->children[1]));
if(is_primitive(root->children[0])==is_primitive(root->children[1])){
  root->attributes[ATTR_bool] = true;
  root->attributes[ATTR_vreg] = true;
  }else{
  //printf("%s\n", root->lexinfo->c_str());
  errprintf("type error, can't compare non-primitives\n");
  }
  
}

void visitTree (astree* root){
   //printf("in visitTree\n");
   // for( auto it = root->children.begin();
  //it != root->children.end();++it){
   //  visitTree(*it);
   //  }
   for (size_t child = 0; child < root->children.size(); ++child) {
                //printf("in loop\n");
                visitTree (root->children[child]);
   }
     
    switch(root->symbol){
      
    
        case TOK_INT: {
         root->attributes[ATTR_int] = true;
        break;
         }
         
        case TOK_CHAR: {
         root->attributes[ATTR_char] = true;
        break;
         }
        
        case TOK_BOOL: {
         root->attributes[ATTR_bool] = true;
        break;
         }
   
        
         
        case TOK_VARDECL: {
          //  printf("in vardecl");
          //compare root->children[0] and root->children[1]
          //if they are the same type
          
            astree* decl = root->children[0];
            astree* cont = root->children[1];
            if(cont->attributes[ATTR_array]){
             if(decl->attributes[ATTR_array]){
              if(strcmp(cont->children[0]->lexinfo->c_str(),
                decl->children[0]->lexinfo->c_str())==0){
            cont->attributes[is_base(decl->children[0])]=true;
            }else{
            //printf("array not the same basetype");
            } 
            }else{
            printf("invalid array declaration\n");       
            }
            }
    
        
            //also compatible if one is reference and one is null
            if(is_ref(root->children[0])&&root->children[1]
              ->attributes[ATTR_null]){
            break;
            }
            
            if(decl->attributes[ATTR_int]!=cont->attributes[ATTR_int]
            &&decl->attributes[ATTR_char]!=cont->attributes[ATTR_char]
            &&decl->attributes[ATTR_bool]!=cont->attributes[ATTR_bool])
            {
            //print error: not the same type
              //errprintf("type error\n");
            }       
          break;
        }
  
  case TOK_WHILE :{
  if(!root->children[0]->attributes[ATTR_bool]){
  //error the expression must be type bool
    //errprintf("type error\n");
  }
  break;
  }
  
  case TOK_IF :{
   if(!root->children[0]->attributes[ATTR_bool]){
  //error the expression must be type bool
    //errprintf("type error\n");
  }
  break;
  }

  
  case '=' :{
        if(root->children[0]->attributes[ATTR_lval]){
              if(is_any(root->children[0])==is_any(root->children[1])){
              root->attributes[is_any(root->children[0])] = true;
              root->attributes[ATTR_vreg] = true;
         }
        } 
 
  break;
  }
  
  case TOK_EQ :{
  if(is_any(root->children[0])==is_any(root->children[1])){
  root->attributes[ATTR_bool] = true;
  root->attributes[ATTR_vreg] = true;
  }
  break;
  }
  
  case TOK_NE :{
  if(is_any(root->children[0])==is_any(root->children[1])){
  root->attributes[ATTR_bool] = true;
  root->attributes[ATTR_vreg] = true;
  }
  break;
  }
  
  
  
  case TOK_LT :{
  check_compar(root);
  break;
  }
  
  case TOK_LE :{
  check_compar(root);
  break;
  }
  
  case TOK_GT :{
  check_compar(root);
  break;
  }
  
  case TOK_GE :{
  check_compar(root); 
  break;
  }
 
  case '+' :{
  check_oper(root);
  break;
  }
  
  case '-' :{
  check_oper(root);
  break;
  }

  case '*' :{
  check_oper(root);
  break;
  }
  
  case '/' :{
  check_oper(root);
  break;
  }
  
  case '%' :{
  check_oper(root); 
  break;
  }
  
  case '.' :{
    root->attributes = root->children[1]->attributes;
  }

  case TOK_POS :{
   if(root->children[0]->attributes[ATTR_int]){
  root->attributes[ATTR_int] = true;
  root->attributes[ATTR_vreg] = true;
  }
  break;
  }
  
  case TOK_NEG: {
   if(root->children[0]->attributes[ATTR_int]){
  root->attributes[ATTR_int] = true;
  root->attributes[ATTR_vreg] = true;
  }
  break;
  }
  
  case '!' :{
  if(root->children[0]->attributes[ATTR_bool]){
  root->attributes[ATTR_bool] = true;
  root->attributes[ATTR_vreg] = true;
  }
  break;
  }
  
  case TOK_ORD :{
  if(root->children[0]->attributes[ATTR_char]){
  root->attributes[ATTR_int] = true;
  root->attributes[ATTR_vreg] = true;
  }
  break;
  }
  
  case TOK_CHR :{
  if(root->children[0]->attributes[ATTR_int]){
  root->attributes[ATTR_char] = true;
  root->attributes[ATTR_vreg] = true;
  }
  break;
  }
  
 case TOK_NEW :{
   root->type = root->children[0]->type;
   root->attributes[ATTR_vreg] = true;
   break;
   }
   
  case TOK_NEWSTRING :{
   if(root->children[0]->attributes[ATTR_int]){
   root->attributes[ATTR_string] = true;
   root->attributes[ATTR_vreg] = true;
   }
   break;
   }
   
  case TOK_NEWARRAY :{
   if(root->children[1]->attributes[ATTR_int]){
   root->attributes[ATTR_array] = true;
   root->attributes[ATTR_vreg] = true;
   }else{
    //errprintf("type error\n");
   //error in new array[int]
   }
   break;
   }
  
  case TOK_ARRAY :{
    if (root->children.size()>1){
      if (root->children[1]->attributes[ATTR_void]){
       errprintf("invalid array type\n");
      }
    }
   root->children[1]->attributes = 
   (root->children[0]->attributes | root->children[1]->attributes);
   if(is_base(root->children[0])){
    //if(root->children[1]->attributes[ATTR_int]){
     root->attributes[ATTR_array] = true;
     root->attributes[is_base(root->children[0])] = true;
     root->attributes[ATTR_vreg] = true;
   
   }
   break;
   }

   // case TOK_ARRAY:{
   //    root->blocknr = blockStack.back();
   //    root->children[1]->attributes[ATTR_array]=true;
   //    root->children[1]->attributes[is_base(root->children[0])]=true;
   //    root->attributes[is_base(root->children[0])]=true;
   //    //root->children[1]->attributes[ATTR_typeid]=true;
   //    //root->children[0]->attributes[ATTR_typeid]=true;
   //    break;
   //  }
   
         
        case TOK_STRING: {
         root->attributes[ATTR_string] = true;
        break;
         }
         
        case TOK_VOID: {
          if (root->children.size()>0){
          if (root->children[0]->attributes[ATTR_variable])
            errprintf("error, void variable\n");
          }
         root->attributes[ATTR_void] = true;
        break;
         } 
         
        
        case TOK_INTCON: {
         root->attributes[ATTR_int] = true;
         root->attributes[ATTR_const] = true;
        break;
        }
        
        case TOK_CHARCON: {
         root->attributes[ATTR_char] = true;
         root->attributes[ATTR_const] = true;
        break;
        }
        
        case TOK_STRINGCON: {
         root->attributes[ATTR_string] = true;
         root->attributes[ATTR_const] = true;
        break;
        }       
  
        case TOK_FALSE: {
         root->attributes[ATTR_bool] = true;
         root->attributes[ATTR_const] = true;
        break;
        }       
       
        case TOK_TRUE: {
        root->attributes[ATTR_bool] = true;
         root->attributes[ATTR_const] = true;
        break;
        }
        
         case TOK_NULL: {
         root->attributes[ATTR_null] = true;
         root->attributes[ATTR_const] = true;
        break;
        }
        
         case TOK_INDEX: {
         root->attributes[ATTR_vaddr]=true;
         root->attributes[ATTR_lval]=true;
         root->attributes[is_base(root->children[0])] = true;
         break;
         }
         
         case TOK_CALL: {
          root->attributes = root->children[0]->attributes;
         }

        default :{//printf("default\n");
        }
    }

}


RCSC("$Id: astree.cpp,v 1.2 2014-12-11 08:18:38-08 - - $")

