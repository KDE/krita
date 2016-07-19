#! /bin/sh
source ../../krita_xgettext.sh

$EXTRACTRC `find . -name \*.ui` >> rc.cpp

krita_xgettext kritacrashhandler.pot  *.cpp
