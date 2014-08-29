#! /bin/sh
source ../../../calligra_xgettext.sh

$EXTRACTRC *.rc *.ui >> rc.cpp
calligra_xgettext sheetspivottables_plugin.pot *.cpp
rm -f rc.cpp
