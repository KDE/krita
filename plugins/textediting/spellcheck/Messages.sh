#! /bin/sh
source ../../../calligra_xgettext.sh

$EXTRACTRC --tag=string *.ui >> rc.cpp
calligra_xgettext SpellCheckPlugin.pot *.cpp
rm -f rc.cpp
