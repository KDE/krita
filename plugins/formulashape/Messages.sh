#! /bin/sh
$EXTRACTRC *.ui  >> rc.cpp
$XGETTEXT *.cpp *.h elements/*.cpp elements/*.h -o $podir/FormulaShape.pot
rm -f rc.cpp
