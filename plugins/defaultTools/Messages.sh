#! /bin/sh
source ../../calligra_xgettext.sh

$EXTRACTRC `find . -name \*.ui` >> rc.cpp
calligra_xgettext *.cpp */*.cpp > $podir/calligra-defaulttools.pot
