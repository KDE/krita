#! /bin/sh
source ../../../calligra_xgettext.sh

$EXTRACTRC *.ui >> rc.cpp
calligra_xgettext AutocorrectPlugin.pot *.cpp
rm -f rc.cpp
