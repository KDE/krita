#! /bin/sh
source ../../../calligra_xgettext.sh

$EXTRACTRC *.rc *.ui >> rc.cpp
calligra_xgettext sheetspivottables.pot *.cpp
rm -f rc.cpp
