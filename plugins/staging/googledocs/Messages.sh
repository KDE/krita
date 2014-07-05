#! /bin/sh
source ../../../calligra_xgettext.sh

$EXTRACTRC *.rc *.ui >> rc.cpp
calligra_xgettext *.cpp > $podir/googledocs_plugin.pot
rm -f rc.cpp
