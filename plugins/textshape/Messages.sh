#! /bin/sh
source ../../krita_xgettext.sh

$EXTRACTRC `find . -name \*.ui` >> rc.cpp || exit 11
krita_xgettext krita_shape_text.pot `find . -name \*.cpp`
rm -f rc.cpp
