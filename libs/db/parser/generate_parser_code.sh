#!/bin/sh
# generates parser and lexer code using bison and flex

builddir=$PWD
srcdir=`dirname $0`
cd $srcdir
flex -osqlscanner.cpp sqlscanner.l
bison -d sqlparser.y -Wall -fall -rall -t --report-file=$builddir/sqlparser.output
echo '#ifndef _SQLPARSER_H_
#define _SQLPARSER_H_
#include <db/field.h>
#include "parser.h"
#include "sqltypes.h"

bool parseData(KexiDB::Parser *p, const char *data);
const char* tokenName(unsigned int offset);
unsigned int maxToken();' > sqlparser.h

function fixWhitespace() {
    sed --in-place 's/[[:space:]]\+$//;s/\t/        /g' $1
}
cat sqlparser.tab.h >> sqlparser.h
echo '#endif' >> sqlparser.h
fixWhitespace sqlparser.h

cat sqlparser.tab.c | sed -e "s/sqlparser\.tab\.c/sqlparser.cpp/g" > sqlparser.cpp
echo 'const char* tokenName(unsigned int offset) { return yytname[YYTRANSLATE(offset)]; }
unsigned int maxToken() { return YYMAXUTOK; }' >> sqlparser.cpp
fixWhitespace sqlparser.cpp

./extract_tokens.sh > tokens.cpp
rm -f sqlparser.tab.h sqlparser.tab.c
