#! /bin/sh
source ../../../calligra_xgettext.sh

$EXTRACTRC `find . -name \*.ui` >> rc.cpp
calligra_xgettext `find . -name \*.cpp` > $podir/calligra_semanticitem_event.pot
