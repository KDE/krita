#! /bin/sh
$EXTRACTRC *.rc *.ui >> rc.cpp
$XGETTEXT *.cpp -o $podir/sheetspivottables_plugin.pot
rm -f rc.cpp
