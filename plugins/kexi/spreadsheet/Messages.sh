#! /bin/sh
$EXTRACTRC *.rc *.ui >> rc.cpp
$XGETTEXT `find . -name \*.cpp ` -o $podir/KexiSpreadsheetImportPlugin.pot
rm -f rc.cpp
