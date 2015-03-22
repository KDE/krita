#! /bin/sh
source ../../calligra_xgettext.sh

$EXTRACTRC *.ui >> rc.cpp
calligra_xgettext calligra_shape_formular.pot *.cpp *.h elements/*.cpp elements/*.h
rm -f rc.cpp
