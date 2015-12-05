#! /bin/sh
source ../../krita_xgettext.sh

$EXTRACTRC `find . -name \*.ui` >> rc.cpp || exit 11
krita_xgettext krita_shape_paths.pot `find . -name \*.cpp -o -name \*.cc`
rm -f rc.cpp
