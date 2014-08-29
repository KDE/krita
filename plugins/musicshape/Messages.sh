#! /bin/sh
source ../../calligra_xgettext.sh

$EXTRACTRC `find . -name \*.ui` >> rc.cpp || exit 11
calligra_xgettext MusicShape.pot `find . -name \*.cpp -o -name \*.cc`
rm -f rc.cpp
