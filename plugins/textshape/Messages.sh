#! /bin/sh
source ../../calligra_xgettext.sh

$EXTRACTRC `find . -name \*.ui` >> rc.cpp || exit 11
calligra_xgettext `find . -name \*.cpp` > $podir/TextShape.pot
rm -f rc.cpp
