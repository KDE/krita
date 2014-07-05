#! /bin/sh
source ../../calligra_xgettext.sh

$EXTRACTRC `find . -name \*.ui` >> rc.cpp || exit 11
calligra_xgettext `find . -name \*.cpp -o -name \*.cc` > $podir/PathShapes.pot
