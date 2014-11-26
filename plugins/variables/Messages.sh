#! /bin/sh
source ../../calligra_xgettext.sh

$EXTRACTRC *.ui >> rc.cpp
calligra_xgettext calligra_textinlineobject_variables.pot *.cpp *.ui
rm -f rc.cpp
