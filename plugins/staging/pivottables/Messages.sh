#! /bin/sh
$EXTRACTRC *.rc *.ui >> rc.cpp
$XGETTEXT -kkundo2_i18nc:1c,2 -kkundo2_i18ncp:1c,2,3 *.cpp -o $podir/sheetspivottables_plugin.pot
rm -f rc.cpp
