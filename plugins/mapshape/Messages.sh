#! /bin/sh
$EXTRACTRC *.rc *.ui >> rc.cpp
$XGETTEXT `find . -name \*.cpp ` -o $podir/MapShape.pot
rm -f rc.cpp
