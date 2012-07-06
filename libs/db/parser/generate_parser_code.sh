#!/bin/sh
# generates parser and lexer code using bison and flex

dir=`dirname $0`
cd $dir
flex -osqlscanner.cpp sqlscanner.l
bison -dv sqlparser.y
echo '#ifndef _SQLPARSER_H_
#define _SQLPARSER_H_
#include <predicatedb/field.h>
#include "parser.h"
#include "sqltypes.h"

bool parseData(KexiDB::Parser *p, const char *data);' > sqlparser.h

cat sqlparser.tab.h >> sqlparser.h
echo '#endif' >> sqlparser.h

cat sqlparser.tab.c | sed -e "s/sqlparser\.tab\.c/sqlparser.cpp/g" > sqlparser.cpp
echo "const char* tname(int offset) { return yytname[offset]; }" >> sqlparser.cpp

./extract_tokens.sh > tokens.cpp
rm -f sqlparser.tab.h sqlparser.tab.c
