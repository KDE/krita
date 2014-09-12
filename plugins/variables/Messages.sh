#! /bin/sh
source ../../calligra_xgettext.sh

$EXTRACTRC *.ui >> rc.cpp
calligra_xgettext VariablesPlugin.pot *.cpp *.ui
rm -f rc.cpp
