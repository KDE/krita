#! /bin/sh
$EXTRACTRC *.ui dialogs/*.ui  >> rc.cpp
$XGETTEXT *.cpp dialogs/*.cpp -o $podir/ChartShape.pot
