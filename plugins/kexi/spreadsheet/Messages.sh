#! /bin/sh
source ../../../calligra_xgettext.sh

$EXTRACTRC *.rc *.ui >> rc.cpp
calligra_xgettext keximigrate_spreadsheet.pot `find . -name \*.cpp`
rm -f rc.cpp
