#! /bin/sh
source ../../calligra_xgettext.sh

$EXTRACTRC `find . -name \*.ui` >> rc.cpp
calligra_xgettext krita_flaketools.pot *.cpp */*.cpp
rm -f rc.cpp
