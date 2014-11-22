#! /bin/sh
source ../../../calligra_xgettext.sh

$EXTRACTRC --tag=string *.ui >> rc.cpp
calligra_xgettext calligra_textediting_spellcheck.pot *.cpp
rm -f rc.cpp
