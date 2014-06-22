#! /bin/sh
source ../../calligra_xgettext.sh

XGETTEXT=${XGETTEXT_QT} calligra_xgettext *.cpp *.h > $podir/kdgantt.pot
