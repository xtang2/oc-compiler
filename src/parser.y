//Andrew Guttman
//asguttma
//Xiaoli Tang
//xtang2

%{

#include "lyutils.h"
#include "astree.h"

%}

%debug
%defines
%error-verbose
%token-table
%verbose

%token TOK_VOID TOK_BOOL TOK_CHAR TOK_INT TOK_STRING
%token TOK_IF TOK_ELSE TOK_WHILE TOK_RETURN TOK_STRUCT
%token TOK_FALSE TOK_TRUE TOK_NULL TOK_NEW TOK_ARRAY
%token TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE
%token TOK_IDENT TOK_INTCON TOK_CHARCON TOK_STRINGCON

%token TOK_BLOCK TOK_CALL TOK_IFELSE TOK_INITDECL
%token TOK_POS TOK_NEG TOK_NEWARRAY TOK_TYPEID TOK_FIELD
%token TOK_ORD TOK_CHR TOK_ROOT

%token TOK_NEWSTRING TOK_INDEX TOK_RETURNVOID TOK_VARDECL TOK_DECLID
%token TOK_PARAMLIST TOK_FUNCTION TOK_PROTOTYPE

%nonassoc "then"
%right TOK_IF TOK_ELSE TOK_IFELSE
%right '='
%left TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE
%left '+' '-'
%left '*' '/' '%'
%right TOK_POS TOK_NEG '!' TOK_ORD TOK_CHR
%left '.' '['
%right UNARY
%left VAR
%nonassoc ALLOC
%nonassoc CALL

%start start
%%

start       : program {yyparse_astree=$1;}

program     : program structdef {$$=adopt1($1,$2);}
            | program function  {$$=adopt1($1,$2);}
            | program statement {$$=adopt1($1,$2);}
            | program error '}' {$$=$1}
            | program error ';' {$$=$1}
            |                   {$$=new_parseroot();}
            ;

structdef   : fielddecls '}'               {$$=$1; }
            | TOK_STRUCT TOK_IDENT '{' '}' {$$=adopt1
               ($1,changesym($2,TOK_TYPEID));}
            ;

fielddecls  : fielddecls fielddecl ';' {$$=adopt1($1,$2);}  
            | TOK_STRUCT TOK_IDENT '{' fielddecl ';' {$$=adopt2
               ($1,changesym($2,TOK_TYPEID),$4);}
            ;

fielddecl   : basetype TOK_ARRAY TOK_IDENT {$$=adopt2($2,$1,
                changesym($3,TOK_FIELD));} 
            | basetype TOK_IDENT           {$$=adopt1($1,
               changesym($2,TOK_FIELD));}
            ;

basetype    : TOK_VOID   {$$=$1;}
            | TOK_BOOL   {$$=$1;}
            | TOK_CHAR   {$$=$1;}
            | TOK_INT    {$$=$1;}
            | TOK_STRING {$$=$1;}
            | TOK_IDENT  {$$=changesym($1, TOK_TYPEID);}
            ;

function    : identdecl funchead ')' block
               {astree* func_root=new_funcroot($1);
               $$=adopt2(func_root,$1,$2);
               $$=adopt1(func_root, $4);}
            | identdecl '(' ')' block
               {astree* func_root = new_funcroot($1);
               $$=adopt2(func_root,$1,changesym($2,TOK_PARAMLIST));
               $$=adopt1(func_root, $4);}
            | identdecl funchead ')' ';'
               {astree* proto_root = new_funcroot($1);
               $$=adopt2(changesym(proto_root,TOK_PROTOTYPE),$1,$2);}
            | identdecl '(' ')' ';'
               {astree* proto_root = new_funcroot($1);
               $$=adopt2(changesym(proto_root,TOK_PROTOTYPE),$1,
               changesym($2,TOK_PARAMLIST));}

            ;


funchead    : funchead ',' identdecl {$$=adopt1($1,$3);}
            | '(' identdecl {$$=adopt1(changesym($1,TOK_PARAMLIST),
               $2);}
            ;

statement   : block    {$$=$1;}
            | vardecl  {$$=$1;}
            | while    {$$=$1;}
            | ifelse   {$$=$1;}
            | return   {$$=$1;}
            | expr ';' {$$=$1;}
            | ';'      {$$=$1;}
            ;

block       : statements '}' {$$=$1}
            | '{' '}' {$$=changesym($1,TOK_BLOCK);}
            ;

statements  : statements statement {$$=adopt1($1,$2);}
            | '{' statement        {$$=adopt1(changesym
               ($1,TOK_BLOCK),$2);}
            ;

vardecl     : identdecl '=' expr ';' {$$=adopt2
               (changesym($2,TOK_VARDECL),$1, $3);}
            ;

identdecl   : basetype TOK_IDENT           {$$=adopt1($1,
               changesym($2,TOK_DECLID));}
            | basetype TOK_ARRAY TOK_IDENT {$$=adopt2($2,$1,
               changesym($3,TOK_DECLID));}
            ;

while       : TOK_WHILE '(' expr ')' statement {$$=adopt2($1,$3,$5);}
            ;

ifelse      : TOK_IF '(' expr ')' statement TOK_ELSE statement
               {$$=adopt2($1, $3, $5);
               $$=adopt1(changesym($1,TOK_IFELSE),$7);}
            | TOK_IF '(' expr ')' statement
               {$$=adopt2($1,$3,$5);} %prec "then"
            ;

return      : TOK_RETURN ';'      {$$=changesym($1,TOK_RETURNVOID);}
            | TOK_RETURN expr ';' {$$=adopt1($1,$2);}
            ;

expr        : expr '=' expr            {$$=adopt2($2,$1,$3);}
            | expr '+' expr            {$$=adopt2($2,$1,$3);}
            | expr '-' expr            {$$=adopt2($2,$1,$3);}
            | expr '*' expr            {$$=adopt2($2,$1,$3);}
            | expr '/' expr            {$$=adopt2($2,$1,$3);}
            | expr '%' expr            {$$=adopt2($2,$1,$3);}
            | expr TOK_EQ expr         {$$=adopt2($2,$1,$3);}
            | expr TOK_NE expr         {$$=adopt2($2,$1,$3);}
            | expr TOK_LT expr         {$$=adopt2($2,$1,$3);}
            | expr TOK_LE expr         {$$=adopt2($2,$1,$3);}
            | expr TOK_GT expr         {$$=adopt2($2,$1,$3);}
            | expr TOK_GE expr         {$$=adopt2($2,$1,$3);}
            | '+' expr %prec UNARY     {$$=adopt1
               (changesym($1,TOK_POS),$2);}
            | '-' expr %prec UNARY     {$$=adopt1
               (changesym($1,TOK_NEG),$2);}
            | '!' expr %prec UNARY     {$$=adopt1($1,$2);}
            | TOK_ORD expr %prec UNARY {$$=adopt1($1,$2);}
            | TOK_CHR expr %prec UNARY {$$=adopt1($1,$2);}
            | allocator %prec ALLOC    {$$=$1;}
            | call %prec CALL          {$$=$1;}
            | variable        {$$=$1;}
            | constant                 {$$=$1;}
            | '(' expr ')'             {$$=$2;}
            ;

allocator   : TOK_NEW TOK_IDENT '(' ')'       {$$=adopt1($1,
               changesym($2,TOK_TYPEID));}
            | TOK_NEW basetype '[' expr ']'   {$$=adopt2(changesym($1,
               TOK_NEWARRAY),changesym($2,TOK_TYPEID),$4);}
            | TOK_NEW TOK_STRING '(' expr ')' {$$=adopt1(changesym($1,
               TOK_NEWSTRING),$4);}
            ;

call        : TOK_IDENT '(' ')' {$$=adopt1(changesym($2,TOK_CALL),
               $1);}
            | exprs ')'         {$$=$1;}
            ;

exprs       : exprs ',' expr     {$$=adopt1($1,$3);}
            | TOK_IDENT '(' expr {$$=adopt2(changesym($2,TOK_CALL),
               $1,$3);}
            ;

variable    : TOK_IDENT          {$$=$1;}
            | expr '[' expr ']' {$$=adopt2(changesym($2,TOK_INDEX),$1,
               $3);}
            | expr '.' TOK_IDENT {$$=adopt2($2, $1, changesym($3,
               TOK_FIELD));}
            ;

constant    : TOK_INTCON    {$$=$1;}
            | TOK_CHARCON   {$$=$1;}
            | TOK_STRINGCON {$$=$1;}
            | TOK_FALSE     {$$=$1;}
            | TOK_TRUE      {$$=$1;}
            | TOK_NULL      {$$=$1;}
            ;

%%

const char *get_yytname (int symbol) {
   return yytname [YYTRANSLATE (symbol)];
}


bool is_defined_token (int symbol) {
   return YYTRANSLATE (symbol) > YYUNDEFTOK;
}

//static void* yycalloc (size_t size) {
//   void* result = calloc (1, size);
//   assert (result != NULL);
//   return result;
//}

