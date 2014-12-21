#! /bin/sh
source ../../calligra_xgettext.sh

$EXTRACTRC *.ui >> rc.cpp
calligra_xgettext calligra_shape_artistictext.pot *.cpp *.h
rm -f rc.cpp
