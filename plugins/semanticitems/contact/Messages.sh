#! /bin/sh
$EXTRACTRC `find . -name \*.ui` >> rc.cpp
$XGETTEXT `find . -name \*.cpp ` -o $podir/calligra_semanticitem_contact.pot
