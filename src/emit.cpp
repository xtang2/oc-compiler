#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include "yyparse.h"
#include "emit.h"
#include "symboltable.h"

FILE* outfile;
int string_count;
int int_count;
int byte_count;
int pointer_count;

const char* indent = "        ";

void emit_while(astree*);
void emit_if(astree*);
void emit_ifelse(astree*);
void emit_return(astree*);
void emit_block(astree*);
void emit_return(astree*);
void emit_returnvoid(astree*);

char* emit_type(astree* node){
   char* return_val = NULL;
   switch (node->symbol) {
      case TOK_TYPEID:
         asprintf(&return_val, "struct s_%s",
                 node->lexinfo->c_str());
         break;
      case TOK_BOOL:
      case TOK_CHAR:
      case TOK_STRING:
         asprintf(&return_val, "char");
         break;
         
      case TOK_INT:
         asprintf(&return_val, "int");
         break;
         
      case TOK_VOID:
         asprintf(&return_val, "void");
         break;
         
      default:
         break;
         
   }
   return return_val;
   
}


char* emit_varname(astree* node, astree* parent, bool is_left){
   char* return_val = "";
   if (node->symbol == TOK_ARRAY) {
      node = node->next;
   }
   if (!is_left) {
      if (node->attributes[ATTR_array]) {
         asprintf(&return_val, "%s*",return_val);
      }
      if (node->attributes[ATTR_string]
        ||node->attributes[ATTR_struct]) {
         asprintf(&return_val, "%s*",return_val);
      }
   }
   if (node->attributes[ATTR_field]) {
      assert(parent != NULL);
      asprintf(&return_val, "%sf_%s_",
               return_val, parent->lexinfo->c_str());
   } else if ((node->blocknr == 0 && !(node->attributes[ATTR_struct]))
             ||(node->declblocknr == 0 
              && !(node->attributes[ATTR_struct]))){
      asprintf(&return_val, "%s__",return_val);
   } else if (node->blocknr != 0){
     if (node->declblocknr != -1){
      asprintf(&return_val, "%s_%d_", return_val, node->declblocknr);
     }else{
      asprintf(&return_val, "%s_%lu_", return_val, node->blocknr);
     }
   }
   asprintf(&return_val, "%s%s", return_val,
            node->lexinfo->c_str());
   return return_val;
}

void emit_structs(astree* root){
   astree* curr = root->children[0];
   astree* field = NULL;
   for (;curr != NULL; curr = curr->next){
      if (curr->symbol == TOK_STRUCT) {
         fprintf(outfile, "%s {\n",
                  emit_type(curr->children[0]));
         for (field = curr->children[0]->next;
              field != NULL; field = field->next) {
            fprintf(outfile, "%s%s %s",
                     indent,
                     emit_type(field),
                     emit_varname(field->children[0],
                      curr->children[0], false));
            fprintf(outfile,";\n");
         }
         fprintf(outfile, "};\n");
      }
   }
}

void emit_strings_rec(astree* node){
   if (node == NULL){
      return;
   }
   if (node->children.size()==0) return;
   for (astree* curr = node->children[0]; curr != NULL;
        curr = curr->next) {
      emit_strings_rec(curr);
   }
   if ((node->attributes[ATTR_string]) &&
         (node->attributes[ATTR_const])) {
      fprintf(outfile, "char *s%d = %s;\n",++string_count,
              node->lexinfo->c_str());
      node->reg = string_count;
   }
}

void emit_strings(astree* root){
   string_count = 0;
   emit_strings_rec(root);
   fprintf(outfile, "\n");
}

void emit_globals(astree* root){
   astree* curr = root->children[0];
   for (;curr != NULL; curr = curr->next){
      if (curr->symbol ==  TOK_VARDECL) {
         fprintf(outfile, "%s %s;",
                  emit_type(curr->children[0]),
                 emit_varname(curr->children[0]->children[0],
                  NULL, false));
         fprintf(outfile, "\n");         
      }
   }
   //fprintf(outfile, "\n");
}

void emit_reg_int(int count){
   fprintf(outfile, "i%d",count);
}

void emit_reg_byte(int count){
   fprintf(outfile, "b%d",count);
}

void emit_reg_pointer(int count){
   fprintf(outfile, "p%d",count);
}

char* emit_binop(char* oper, char* left, char* right){
   char* out;
   asprintf(&out, "b%d", ++byte_count);
   fprintf(outfile, "%schar %s = %s %s %s;\n", indent, out, left,
           oper, right);
   return out;
}

char* emit_int_unop(char* oper, char* expr){
   char* out;
   asprintf(&out, "i%d", ++int_count);
   fprintf(outfile, "%sint %s = %s %s;\n", indent, out, oper, expr);
   return out;
}

char* emit_byte_unop(char* oper, char* expr){
   char* out;
   asprintf(&out, "b%d", ++byte_count);
   fprintf(outfile, "%schar %s = %s %s;\n", indent, out, oper, expr);
   return out;
}

char* emit_expr(astree* expr){
   char* return_val = NULL;
   const char* left = NULL;
   char* right = NULL;
   right = right;
   char** call_args = NULL;
   int arg_count = 0;
   
   switch (expr->symbol) {
      case '=':
         fprintf(outfile, "%s%s %c %s;\n", indent,
                 emit_expr(expr->children[0]),
                 expr->symbol, emit_expr(expr->children[0]->next));
         break;
      case '+':
      case '-':
      case '*':
      case '/':
      case '%':
         asprintf(&return_val, "i%d", ++int_count);
         fprintf(outfile, "%sint %s = %s %c %s;\n", indent,
                 return_val, emit_expr(expr->children[0]),
                 expr->symbol, emit_expr(expr->children[0]->next));
         break;
         
      case TOK_EQ:
         return_val = emit_binop("==",emit_expr(expr->children[0]),
                    emit_expr(expr->children[0]->next));
         break;
      case TOK_NE:
         return_val = emit_binop("!=",emit_expr(expr->children[0]),
                    emit_expr(expr->children[0]->next));
         break;
      case TOK_LT:
         return_val = emit_binop("<",emit_expr(expr->children[0]),
                    emit_expr(expr->children[0]->next));
         break;
      case TOK_LE:
         return_val = emit_binop("<=",emit_expr(expr->children[0]),
                    emit_expr(expr->children[0]->next));
         break;
      case TOK_GT:
         return_val = emit_binop(">",emit_expr(expr->children[0]),
                    emit_expr(expr->children[0]->next));
         break;
      case TOK_GE:
         return_val = emit_binop(">=",emit_expr(expr->children[0]),
                    emit_expr(expr->children[0]->next));
         break;
         
      case TOK_POS:
         return_val = emit_int_unop("+", emit_expr(expr->children[0]));
         break;
      case TOK_NEG:
         return_val = emit_int_unop("-", emit_expr(expr->children[0]));
         break;
      case '!':
         return_val = emit_byte_unop("+",
          emit_expr(expr->children[0]));
         break;
      case TOK_ORD:
         return_val = emit_int_unop("(int)",
           emit_expr(expr->children[0]));
         break;
      case TOK_CHR:
         return_val = emit_byte_unop("(char)",
           emit_expr(expr->children[0]));
         break;

      case TOK_NEW:
         asprintf(&return_val, "p%d", ++pointer_count);
         fprintf(outfile,
          "%sstruct %s *%s = xcalloc (1, sizeof (struct %s));\n",
                 indent,
                 expr->children[0]->lexinfo->c_str(),
                 return_val,
                 expr->children[0]->lexinfo->c_str());
         break;
         
      case TOK_NEWSTRING:
         asprintf(&return_val, "p%d", ++pointer_count);
         fprintf(outfile,
          "%schar *%s = xcalloc (%s, sizeof (char));\n",
                 indent,
                 return_val,
                 emit_expr(expr->children[0]->next));
         break;
         
      case TOK_NEWARRAY:
          if ((expr->children[0]->attributes[ATTR_array])
                   && (expr->children[0]->attributes [ATTR_string])){
            asprintf(&return_val, "p%d", ++pointer_count);
            fprintf(outfile,
                    "%schar **%s = xcalloc (%s, sizeof (char *));\n",
                    indent,
                    return_val,
                    emit_expr(expr->children[0]->next));
         }else if(expr->children[0]->attributes[ATTR_int]){
            asprintf(&return_val, "p%d", ++pointer_count);
            fprintf(outfile,
                    "%sint *%s = xcalloc (%s, sizeof (int));\n",
                    indent,
                    return_val,
                    emit_expr(expr->children[0]->next));
         }else if (expr->children[0]->attributes[ATTR_struct]) {
            asprintf(&return_val, "p%d", ++pointer_count);
            fprintf(outfile,
          "%sstruct %s **%s = xcalloc (%s, sizeof (struct %s *));\n",
                    indent,
                    expr->children[0]->lexinfo->c_str(),
                    return_val,
                    emit_expr(expr->children[0]->next),
                    expr->children[0]->lexinfo->c_str());
         }else  {
            asprintf(&return_val, "p%d", ++pointer_count);
            fprintf(outfile,
                    "%schar *%s = xcalloc (%s, sizeof (char));\n",
                    indent,
                    return_val,
                    emit_expr(expr->children[0]->next));
         }
         break;
         
      case TOK_CALL: {
         for (astree* arg = expr->children[0]->next;
              arg != NULL ; arg = arg->next) {
            ++arg_count;
         }
         call_args = (char**)alloca(arg_count*(sizeof(char*)));
         int i = 0;
         for (astree* arg = expr->children[0]->next;
              arg != NULL; arg = arg->next, i++) {
              asprintf(&call_args[i], "%s", emit_expr(arg));
         }
         if ((expr->children[0]->attributes[ATTR_array])
             && (expr->children[0]->attributes[ATTR_string])) {
            asprintf(&return_val, "b%d", ++byte_count);
            fprintf(outfile, "%schar *%s = __%s (",
                    indent,
                    return_val,
                    expr->children[0]->lexinfo->c_str());
         }else if ((expr->attributes[ATTR_array])
                   && (expr->attributes[ATTR_int])) {
            asprintf(&return_val, "i%d", ++int_count);
            fprintf(outfile, "%sint *%s = __%s (",
                    indent,
                    return_val,
                    expr->children[0]->lexinfo->c_str());
         }else if (expr->attributes[ATTR_array]){
            asprintf(&return_val, "b%d", ++byte_count);
            fprintf(outfile, "%schar *%s = __%s (",
                    indent,
                    return_val,
                    expr->children[0]->lexinfo->c_str());
         }else if (expr->attributes[ATTR_int]) {
            asprintf(&return_val, "i%d", ++int_count);
            fprintf(outfile, "%sint %s = __%s (",
                    indent,
                    return_val,
                    expr->children[0]->lexinfo->c_str());
         }else if (expr->attributes[ATTR_void]) {
            fprintf(outfile, "%s__%s (",
                    indent,
                    expr->children[0]->lexinfo->c_str());
         }else if ((expr->attributes[ATTR_string])){
            asprintf(&return_val, "b%d", ++byte_count);
            fprintf(outfile, "%schar %s = __%s (",
                    indent,
                    return_val,
                    expr->children[0]->lexinfo->c_str());
         } else {
            asprintf(&return_val, "b%d", ++byte_count);
            fprintf(outfile, "%schar %s = __%s (",
                    indent,
                    return_val,
                    expr->children[0]->lexinfo->c_str());
         }
         for (i = 0; i < arg_count; i++) {
            fprintf(outfile, "%s", call_args[i]);
         }
         fprintf(outfile, ");\n");
         break;
     }
         
      case TOK_IDENT:
         asprintf(&return_val, "%s",
                  emit_varname(expr, NULL, true));
         break;
         
      case TOK_INTCON:
         left = expr->lexinfo->c_str();
         if (atoi(left) != 0) {
            while (*left && *left == '0') left++; // remove leading 0's
         }
         asprintf(&return_val, "%d",
                  atoi(left));
         break;
         
      case TOK_CHARCON:
         asprintf(&return_val, "%s",
                  expr->lexinfo->c_str());
         break;
         
      case TOK_STRINGCON:
         asprintf(&return_val, "s%d",
                  expr->reg);
         break;
         
      case TOK_NULL:
      case TOK_FALSE:
         asprintf(&return_val, "0");
         break;
         
      case TOK_TRUE:
         asprintf(&return_val, "1");
         break;
         
      case TOK_ARRAY:
         asprintf(&return_val, "%s[%s]",
                  emit_expr(expr->children[0]),
                  emit_expr(expr->children[0]->next));
         break;
         
      case TOK_FIELD:
         asprintf(&return_val, "%s->%s",
                  emit_expr(expr->children[0]),
                  emit_varname(expr, expr->children[0], true));
         break;
         
      case TOK_BLOCK:
         emit_block(expr);
         break;
         
      case TOK_RETURN:
         emit_return(expr);
         break;
         
      case TOK_RETURNVOID:
         emit_returnvoid(expr);
         break;
         
      default:
         break;
   }
   return return_val;
}

void emit_vardecl(astree* vardecl){
   assert(vardecl->symbol == TOK_VARDECL);
   fprintf(outfile, "%s %s = %s;\n",
           emit_type(vardecl->children[0]),
           emit_varname(vardecl->children[0]->children[0],
            NULL, false),
           emit_expr(vardecl->children[0]->next));
}

void emit_while(astree* root){
   fprintf(outfile, "while_%lu_%lu_%lu:;\n",
           root->filenr,
           root->linenr,
           root->offset);
   char* condition = emit_expr(root->children[0]);
   fprintf(outfile, "%sif (!%s) goto break_%lu_%lu_%lu;\n",
           indent,
           condition,
           root->filenr,
           root->linenr,
           root->offset);
   emit_expr(root->children[0]->next);
   fprintf(outfile, "%sgoto while_%lu_%lu_%lu;\n",
           indent,
           root->filenr,
           root->linenr,
           root->offset);
   fprintf(outfile, "break_%lu_%lu_%lu:;\n",
           root->filenr,
           root->linenr,
           root->offset);
}

void emit_if_else(astree* root){
   fprintf(outfile, "%sif (!%s) goto else_%lu_%lu_%lu;\n",
           indent,
           emit_expr(root->children[0]),
           root->filenr,
           root->linenr,
           root->offset);
   emit_expr(root->children[0]->next);
   fprintf(outfile, "%sgoto fi_%lu_%lu_%lu;\n",
           indent,
           root->filenr,
           root->linenr,
           root->offset);
   fprintf(outfile, "else_%lu_%lu_%lu:;\n",
           root->filenr,
           root->linenr,
           root->offset);
   emit_expr(root->children[0]->next->next);
   fprintf(outfile, "fi_%lu_%lu_%lu:;\n",
           root->filenr,
           root->linenr,
           root->offset);
}

void emit_if(astree* root){
   fprintf(outfile, "%sif (!%s) goto fi_%lu_%lu_%lu;\n",
           indent,
           emit_expr(root->children[0]),
           root->filenr,
           root->linenr,
           root->offset);
   emit_expr(root->children[0]->next);
   fprintf(outfile, "fi_%lu_%lu_%lu:;\n",
           root->filenr,
           root->linenr,
           root->offset);
}

void emit_return(astree* root){
   fprintf(outfile, "%sreturn %s;\n",
           indent,
           emit_expr(root->children[0]));
}

void emit_returnvoid(astree* root){
   fprintf(outfile, "%sreturn;\n",
           indent);
   root=root;
}

void emit_block(astree* block){
   assert( block->symbol == TOK_BLOCK );
   for (astree* curr = block->children[0];
        curr != NULL; curr = curr->next) {
      switch (curr->symbol) {
         case TOK_BLOCK:
            emit_block(curr);
            break;
         case TOK_VARDECL:
            fprintf(outfile, "%s", indent);
            emit_vardecl(curr);
            break;
         case TOK_WHILE:
            emit_while(curr);
            break;
         case TOK_IF:
            emit_if(curr);
            break;
         case TOK_IFELSE:
            emit_if_else(curr);
            break;
         case TOK_RETURN:
            emit_return(curr);
            break;
         case TOK_RETURNVOID:
            emit_returnvoid(curr);
            break;
            
         default:
            emit_expr(curr);
            break;
      }
   }
}

void emit_functions(astree* node){
   if (node==NULL) return;
   if (node->children.size()==0) return;
   for (astree* curr = node->children[0]; curr != NULL;
        curr = curr->next) {
      if (curr->symbol == TOK_FUNCTION) {
         fprintf(outfile, "%s %s (\n",
                  emit_type(curr->children[0]),
                  emit_varname(curr->children[0]->children[0],
                   NULL, false));
      if (curr->children[0]->next->children.size()==0) return;
      else{
         for (astree* arg = curr->children[0]->next->children[0];
          arg != NULL;
              arg = arg->next) {
          if (curr->children[0]->next->children.size()==0) break;
            fprintf(outfile, "%s%s %s",
                     indent,
                     emit_type(arg),
                     emit_varname(arg->children[0], NULL, false));
            if (arg->next != NULL)
               fprintf(outfile, ",\n");
         }
        }
         fprintf(outfile, ")\n");
      }
      if (curr->symbol == TOK_FUNCTION){
         fprintf(outfile, "{\n");
         emit_block(curr->children[0]->next->next);
         fprintf(outfile, "}\n");
      }
   }
}

void emit_main(astree* node){
   fprintf(outfile, "void __ocmain (void) \n{\n");
   for (astree* curr = node->children[0];
        curr != NULL; curr = curr->next ) {
      switch (curr->symbol) {
         case TOK_BLOCK:
            emit_block(curr);
            break;
            
         case TOK_VARDECL:
            fprintf(outfile, "%s%s = %s;\n",
                    indent,
                    emit_varname(curr->children[0]->children[0],
                     NULL, true),
                    emit_expr(curr->children[0]->next));
            break;
            
         case TOK_WHILE:
            emit_while(curr);
            break;
            
         case TOK_IFELSE:
            emit_if_else(curr);
            break;
            
         case TOK_IF:
            emit_if(curr);
            break;
            
         case TOK_RETURN:
            emit_return(curr);
            break;
            
         case TOK_RETURNVOID:
            emit_returnvoid(curr);
            break;
            
         default:
            if (!(curr->symbol == TOK_FUNCTION
                || curr->symbol == TOK_STRUCT)) {
               emit_expr(curr);
            }
            break;
      }
   }
   fprintf(outfile, "}\n");
   
}

void emit_oil(FILE* output_file, astree* root){
   if (root->children.size()==0) return;
   outfile = output_file;
   fprintf(outfile, "#define __OCLIB_C__\n");
   fprintf(outfile, "#include \"oclib.oh\"\n");
   emit_structs(root);
   emit_strings(root);
   emit_globals(root);
   emit_functions(root);
   emit_main(root);
   
}

