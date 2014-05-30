#! /bin/sh
source ../../../calligra_xgettext.sh

$EXTRACTRC *.rc *.ui >> rc.cpp
calligra_xgettext *.cpp > $podir/sheetspivottables_plugin.pot
rm -f rc.cpp
