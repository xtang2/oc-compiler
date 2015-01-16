//Andrew Guttman
//asguttma
//Xiaoli Tang
//xtang2

#ifndef __STRINGSET__
#define __STRINGSET__

#include <string>
#include <unordered_set>
using namespace std;

#include <stdio.h>

#include "auxlib.h"

const string* intern_stringset (const char*);

void dump_stringset (FILE*);

RCSH("$Id: stringset.h,v 1.1 2014-12-10 19:34:13-08 - - $")
#endif
