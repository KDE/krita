#! /bin/sh
source ../../calligra_xgettext.sh

$EXTRACTRC *.ui >> rc.cpp
calligra_xgettext *.cpp *.h > $podir/ArtisticTextShape.pot
