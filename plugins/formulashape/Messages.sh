#! /bin/sh
$EXTRACTRC *.ui  >> rc.cpp
$XGETTEXT -kkundo2_i18nc:1c,2 -kkundo2_i18ncp:1c,2,3 *.cpp *.h elements/*.cpp elements/*.h -o $podir/FormulaShape.pot
rm -f rc.cpp
