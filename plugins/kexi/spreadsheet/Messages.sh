#! /bin/sh
$EXTRACTRC *.rc *.ui >> rc.cpp
$XGETTEXT `find . -name \*.cpp ` -o $podir/keximigrate_spreadsheet.pot
rm -f rc.cpp
