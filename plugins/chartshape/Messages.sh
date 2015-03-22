#! /bin/sh
source ../../calligra_xgettext.sh

$EXTRACTRC `find . -name \*.ui` >> rc.cpp
calligra_xgettext calligra_shape_chart.pot `find . -name \*.cpp`
rm -f rc.cpp
