#! /bin/sh
source ../../../calligra_xgettext.sh

calligra_xgettext `find . -name \*.cpp` > $podir/koreport_barcodeplugin.pot
