#! /bin/sh
#$EXTRACTRC `find . -name \*.ui` > rc.cpp || exit 11
$XGETTEXT `find . -name \*.cpp -o -name \*.cc` -o $podir/PathShapes.pot
