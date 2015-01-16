/*
//Andrew Guttman
//asguttma
//Xiaoli Tang
//xtang2
*/

%{

#include "auxlib.h"
#include "lyutils.h"

#define YY_USER_ACTION  { scanner_useraction (); }
#define IGNORE(THING)   { }

%}

%option 8bit
%option debug
%option nodefault
%option nounput
%option noyywrap
%option verbose
%option warn

LETTER          [A-Za-z_]
DIGIT           [0-9]
IDENT           ({LETTER}({LETTER}|{DIGIT})*)
INTEGER         ({DIGIT}+)
CHAR_CONST      ('([^\\'\n]|\\[\\'"0nt])')
STRING_CONST    (\"([^\\"\n]|\\[\\'"0nt])*\")
INVAL_ID        ({DIGIT}+{LETTER}+({DIGIT}|{LETTER})*)
INVAL_CHAR      (('([^\\'\n]|\\[\\'"0nt]))|('\\')|''|'.+.+')
INVAL_STR       (\"([^\\"\n]|\\[^\\'"0nt])*\")
INVAL_STR2      (\"([^\\"\n]|\\[\\'"0nt])*)

%%

"[]"            { return yylval_token (TOK_ARRAY); }
"("             { return yylval_token ('('); }
")"             { return yylval_token (')'); }
"["             { return yylval_token ('['); }
"]"             { return yylval_token (']'); }
"{"             { return yylval_token ('{'); }
"}"             { return yylval_token ('}'); }
";"             { return yylval_token (';'); }
","             { return yylval_token (','); }
"."             { return yylval_token ('.'); }
"="             { return yylval_token ('='); }
"=="            { return yylval_token (TOK_EQ); }
"!="            { return yylval_token (TOK_NE); }
"<"             { return yylval_token (TOK_LT); }
"<="            { return yylval_token (TOK_LE); }
">"             { return yylval_token (TOK_GT); }
">="            { return yylval_token (TOK_GE); }
"+"             { return yylval_token ('+'); }
"-"             { return yylval_token ('-'); }
"*"             { return yylval_token ('*'); }
"/"             { return yylval_token ('/'); }
"%"             { return yylval_token ('%'); }
"!"             { return yylval_token ('!'); }

"void"          { return yylval_token (TOK_VOID); }
"bool"          { return yylval_token (TOK_BOOL); }
"char"          { return yylval_token (TOK_CHAR); }
"int"           { return yylval_token (TOK_INT); }
"string"        { return yylval_token (TOK_STRING); }
"struct"        { return yylval_token (TOK_STRUCT); }
"if"            { return yylval_token (TOK_IF); }
"else"          { return yylval_token (TOK_ELSE); }
"while"         { return yylval_token (TOK_WHILE); }
"return"        { return yylval_token (TOK_RETURN); }
"false"         { return yylval_token (TOK_FALSE); }
"true"          { return yylval_token (TOK_TRUE); }
"null"          { return yylval_token (TOK_NULL); }
"ord"           { return yylval_token (TOK_ORD); }
"chr"           { return yylval_token (TOK_CHR); }
"new"           { return yylval_token (TOK_NEW); }

{IDENT}         { return yylval_token (TOK_IDENT); }
{INTEGER}       { return yylval_token (TOK_INTCON); }
{CHAR_CONST}    { return yylval_token (TOK_CHARCON); }
{STRING_CONST}  { return yylval_token (TOK_STRINGCON); }

{INVAL_ID}      { scanner_badtoken (yytext);
                  return yylval_token(TOK_IDENT);}
{INVAL_CHAR}    { scanner_badtoken (yytext);
                  return yylval_token(TOK_CHARCON);}   
{INVAL_STR}     { scanner_badtoken (yytext);
                  return yylval_token(TOK_STRINGCON);}
{INVAL_STR2}    { scanner_badtoken (yytext);
                  return yylval_token(TOK_STRINGCON);}


[ \t]+          { IGNORE (white space) }
\n              { scanner_newline(); }
"#".*           { scanner_include(); }





.               { scanner_badchar (*yytext); }

%%





