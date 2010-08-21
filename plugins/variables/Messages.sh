#! /bin/sh
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT *.cpp *.ui -o $podir/VariablesPlugin.pot
