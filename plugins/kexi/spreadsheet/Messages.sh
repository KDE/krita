#! /bin/sh
source ../../../calligra_xgettext.sh

$EXTRACTRC *.rc *.ui >> rc.cpp
calligra_xgettext `find . -name \*.cpp` > $podir/keximigrate_spreadsheet.pot
if [ ! -s $podir/keximigrate_spreadsheet.pot ]; then
    rm -f $podir/keximigrate_spreadsheet.pot
fi

rm -f rc.cpp
