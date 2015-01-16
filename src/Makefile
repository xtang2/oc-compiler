#Andrew Guttman
#asguttma
#Xiaoli Tang
#xtang2

MKFILE    = Makefile
DEPFILE   = ${MKFILE}.deps

HSOURCES  = astree.h lyutils.h auxlib.h stringset.h symboltable.h\
            emit.h
CSOURCES  = astree.cpp lyutils.cpp auxlib.cpp stringset.cpp\
            main.cpp symboltable.cpp emit.cpp
LSOURCES  = scanner.l
YSOURCES  = parser.y
ETCSRC    = README ${MKFILE} ${DEPFILE} PARTNER
CLGEN     = yylex.cpp
HYGEN     = yyparse.h
CYGEN     = yyparse.cpp
CGENS     = ${CLGEN} ${CYGEN}
ALLGENS   = ${HYGEN} ${CGENS}
#EXECBIN   = zexprsm
ALLCSRC   = ${CSOURCES} ${CGENS}
OBJECTS   = ${ALLCSRC:.cpp=.o}
LREPORT   = yylex.output
YREPORT   = yyparse.output
#IREPORT   = ident.output
REPORTS   = ${LREPORT} ${YREPORT}
ALLSRC    = ${YSOURCES} ${LSOURCES} ${HSOURCES} ${CSOURCES}
#TESTINS   = ${wildcard test?.in}
#LISTSRC   = ${ALLSRC} ${HYGEN}

GPP = g++ -g -O0 -Wall -Wextra -std=gnu++11 -Wno-write-strings
MKDEP = g++ -MM -std=gnu++11

all : ${OBJECTS}
	${GPP} ${OBJECTS} -o oc

%.o : %.cpp
	${GPP} -c $<

${CLGEN} : ${LSOURCES}
	flex --outfile=${CLGEN} ${LSOURCES} 2>${LREPORT}
	- grep -v '^  ' ${LREPORT}

${CYGEN} ${HYGEN} : ${YSOURCES}
	bison -v --defines=${HYGEN} --output=${CYGEN} ${YSOURCES}


clean :
	rm ${OBJECTS} ${ALLGENS} ${REPORTS}

spotless : clean
	rm oc

ci :
	checksource ${ALLSRC} ${ETCSRC}
	cid + ${ALLSRC}

${DEPFILE} :
	${MKDEP} ${ALLCSRC} >${DEPFILE}

deps : 
	- rm ${DEPFILE}
	${MAKE} --no-print-directory ${DEPFILE}

include Makefile.deps

submit :
	submit cmps104a-wm.f14 asg5 ${ALLSRC} ${ETCSRC}
