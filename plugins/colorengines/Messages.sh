#! /bin/sh
source ../../calligra_xgettext.sh

calligra_xgettext `find . -name \*.cpp -o -name \*.h` > $podir/kocolorspaces.pot
