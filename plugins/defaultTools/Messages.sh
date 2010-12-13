#! /bin/sh
$EXTRACTRC `find . -name \*.ui` >> rc.cpp
$XGETTEXT *.cpp */*.cpp -o $podir/calligra-defaulttools.pot

