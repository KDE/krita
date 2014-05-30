#! /bin/sh
source ../../calligra_xgettext.sh

$EXTRACTRC *.ui >> rc.cpp
calligra_xgettext *.cpp *.h elements/*.cpp elements/*.h > $podir/FormulaShape.pot
rm -f rc.cpp
