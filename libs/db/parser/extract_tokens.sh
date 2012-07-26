#!/bin/sh

echo "/* WARNING! All changes made in this file will be lost! Run 'generate_parser_code.sh' instead. */
static const char* _tokens[] = {"
for t in `grep  "\"[a-zA-Z_]*\"" sqlscanner.l | sed -e "s/\(^[^\"]*\)\"\([^\"]*\)\".*$/\2/g" | sort | uniq` ; do
	if [ "$t" = "ZZZ" ] ; then break ; fi
	echo "\"$t\",";
done

echo "0
};"
