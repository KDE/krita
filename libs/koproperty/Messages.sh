#! /bin/sh
source $srcdir/../../calligra_xgettext.sh

calligra_xgettext *.cpp editors/*.cpp > $podir/koproperty.pot
