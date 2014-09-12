#! /bin/sh
source ../../calligra_xgettext.sh

$EXTRACTRC *.ui >> rc.cpp
calligra_xgettext ArtisticTextShape.pot *.cpp *.h
rm -f rc.cpp
