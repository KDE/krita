#! /bin/sh
source ../calligra_xgettext.sh

EXCLUDE="-path ./koproperty" # TODO: -o -path ./koreport"
LIST=`find . \( \( $EXCLUDE \) -prune -o -name \*.ui -o -name \*.rc \) -type f | grep -v -e '/\.'`
$EXTRACTRC $LIST >> rc.cpp
LIST=`find . \( \( $EXCLUDE \) -prune -o -name \*.cpp -o -name \*.cc -o -name \*.h \) -type f | grep -v '/tests/' | grep -v -e '/\.'`
calligra_xgettext $LIST > $podir/calligra.pot
rm -f rc.cpp
