#! /bin/sh
source ../../calligra_xgettext.sh

$EXTRACTRC *.ui >> rc.cpp
calligra_xgettext FormulaShape.pot *.cpp *.h elements/*.cpp elements/*.h
rm -f rc.cpp
