#! /bin/sh
source ../../../calligra_xgettext.sh

$EXTRACTRC *.ui >> rc.cpp
calligra_xgettext calligra_textediting_autocorrect.pot *.cpp
rm -f rc.cpp
