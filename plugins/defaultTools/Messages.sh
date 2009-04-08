#! /bin/sh
$EXTRACTRC `find . -name \*.ui` >> rc.cpp
$XGETTEXT *.cpp */*.cpp -o $podir/koffice-defaulttools.pot

