#!/bin/sh
# Generates parser and lexer code using bison and flex

BISON_MIN=3.0.4      # keep updated for best results
BISON_MIN_NUM=30004  # keep updated for best results
FLEX_MIN=2.5.37      # keep updated for best results
FLEX_MIN_NUM=20537   # keep updated for best results

# Check minimum version of bison
bisonv=`bison --version | head -n 1| cut -f4 -d" "`
bisonv1=`echo $bisonv | cut -f1 -d.`
bisonv2=`echo $bisonv | cut -f2 -d.`
bisonv3=`echo $bisonv | cut -f3 -d.`
bisonvnum=`expr $bisonv1 \* 10000 + $bisonv2 \* 100 + $bisonv3`
if [ $bisonvnum -lt $BISON_MIN_NUM ] ; then
    echo "$bisonv is too old bison version, the minimum is $BISON_MIN."
    exit 1
fi

# Check minimum version of flex
flexv=`flex --version | head -n 1| cut -f2 -d" "`
flexv1=`echo $flexv | cut -f1 -d.`
flexv2=`echo $flexv | cut -f2 -d.`
flexv3=`echo $flexv | cut -f3 -d.`
flexvnum=`expr $flexv1 \* 10000 + $flexv2 \* 100 + $flexv3`
if [ $flexvnum -lt $FLEX_MIN_NUM ] ; then
    echo "$flexv is too old flex version, the minimum is $FLEX_MIN."
    exit 1
fi

# Generate lexer and parser
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

# Fine-tune the code: extra functions and remove trailing white space
cat sqlparser.tab.h >> sqlparser.h
echo '#endif' >> sqlparser.h
sed --in-place 's/[[:space:]]\+$//;s/\t/        /g' sqlparser.h

cat sqlparser.tab.c | sed -e "s/sqlparser\.tab\.c/sqlparser.cpp/g" > sqlparser.cpp
echo 'const char* tokenName(unsigned int offset) { return yytname[YYTRANSLATE(offset)]; }
unsigned int maxToken() { return YYMAXUTOK; }' >> sqlparser.cpp
sed --in-place 's/[[:space:]]\+$//;s/\t/        /g' sqlparser.cpp

# Extract a table of SQL tokens
./extract_tokens.sh > tokens.cpp
rm -f sqlparser.tab.h sqlparser.tab.c
