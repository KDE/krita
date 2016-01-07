#! /bin/sh
source ../../krita_xgettext.sh

$EXTRACTRC *.ui >> rc.cpp
krita_xgettext krita_shape_artistictext.pot *.cpp *.h
rm -f rc.cpp
