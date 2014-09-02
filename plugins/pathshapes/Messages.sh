#! /bin/sh
source ../../calligra_xgettext.sh

$EXTRACTRC `find . -name \*.ui` >> rc.cpp || exit 11
calligra_xgettext PathShapes.pot `find . -name \*.cpp -o -name \*.cc`
rm -f rc.cpp
