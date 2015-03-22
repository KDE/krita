#! /bin/sh
source ../../../calligra_xgettext.sh

$EXTRACTRC *.rc *.ui >> rc.cpp
calligra_xgettext googledocs_plugin.pot *.cpp
rm -f rc.cpp
