//Andrew Guttman
//asguttma
//Xaioli Tang
//xtang2

using namespace std;
#include <string>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "stringset.h"
#include "auxlib.h"
#include "lyutils.h"
#include "astree.h"
#include "emit.h"

const string CPP = "/usr/bin/cpp";
//const size_t LINESIZE = 1024;

int exitStatus = 0;
int pclose_rc = 0;

string cppOpt = "";

extern int yy_flex_debug;
extern int yydebug;
extern int yylex(void);
extern int yyparse(void);
extern FILE *yyin;
FILE *tokfile;
FILE *astfile;
FILE *symfile;
FILE *oilfile;


void scan_opts (int argc, char** argv) {
  int option;
  yy_flex_debug = 0;
  //int opterr = 0;
  for(;;) {
    option = getopt (argc, argv, "@:D:ly");
    if (option == EOF) break;
    switch (option) {
      case '@': set_debugflags (optarg);         break;
      case 'D': cppOpt = cppOpt + "-D" + optarg; break;
      case 'l': yy_flex_debug = 1;               break;
      case 'y': yydebug = 1;                     break;
      default: errprintf ("%:bad option (%c)\n", optopt);
         exitStatus = 1; break;
    }
  }
  if (optind > argc) {
    errprintf ("Usage: %s [-ly] [filename]\n", get_execname());
    exitStatus = 1;
  }
}

void chomp (char* string, char delim) {
   size_t len = strlen (string);
   if (len == 0) return;
   char* nlpos = string + len - 1;
   if (*nlpos == delim) *nlpos = '\0';
}

void cppstrtok(string filename) {
//  char buffer[LINESIZE];
  string command = CPP + " " + cppOpt + " " + filename;
  yyin = popen (command.c_str(), "r");
  if (yyin == NULL) {
    syserrprintf (command.c_str());
  }else {
  
   //single call to yyparse()
     int parse = yyparse();
     parse=parse;
//        if(parse){
//            errprintf("...\n");
//            return;
//        }
     processTree(yyparse_astree);
     visitTree(yyparse_astree);
     dump_astree(astfile, yyparse_astree);
     setNext(yyparse_astree);
     emit_oil(oilfile, yyparse_astree);


  }
 
  pclose_rc = pclose (yyin);

  if (pclose_rc != 0) errprintf ("");// exitStatus = 1;
}

int main (int argc, char **argv) {
  set_execname (argv[0]);
  if (argc < 2){
    errprintf("Program requires a \".oc\" file.\n", get_execname());
    return 1;
  }

  string filename = argv[argc - 1];
  string secondToLast = argv[argc-2];

  if(secondToLast.compare("oc") && !(argv[argc-2][0] == '-')) {
    errprintf("Program only accepts a single file at a time.\n");
    exitStatus = 1;
  }

  string outfile = basename(filename.c_str());
  size_t found = outfile.find_last_of(".");
  string suffix = outfile.substr(found+1);
  if (strcmp("oc", suffix.c_str()) != 0) {
    errprintf("Program requires a \".oc\" file.\n", get_execname());
    return 1;
  }
  outfile = outfile.substr(0,found);
  string outfile2 = outfile;
  string outfile3 = outfile;
  string outfile4 = outfile;
  string outfile5 = outfile;
  outfile = outfile.append (".str");
  outfile2.append (".tok");
  outfile3.append (".ast");
  outfile4.append (".sym");
  outfile5.append (".oil");

  scan_opts (argc, argv);
  
  tokfile = fopen (outfile2.c_str(),"w");
  astfile = fopen (outfile3.c_str(),"w");
  symfile = fopen (outfile4.c_str(),"w");
  oilfile = fopen (outfile5.c_str(), "w");

  cppstrtok (filename);

  FILE *out = fopen (outfile.c_str(), "w");
  dump_stringset(out);
  fclose (tokfile);
  fclose (out);
  return get_exitstatus();
}

