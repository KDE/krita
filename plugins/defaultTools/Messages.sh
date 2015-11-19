#! /bin/sh
source ../../krita_xgettext.sh

$EXTRACTRC `find . -name \*.ui` >> rc.cpp
krita_xgettext krita_flaketools.pot *.cpp */*.cpp
rm -f rc.cpp
