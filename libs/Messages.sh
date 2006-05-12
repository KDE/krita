#! /bin/sh
EXCLUDE="-path ./koproperty"
LIST=`find . \( \( $EXCLUDE \) -prune -o -name \*.ui -o -name \*.rc \) -type f | grep -v -e '/\.'`
$EXTRACTRC $LIST > rc.cpp
LIST=`find . \( \( $EXCLUDE \) -prune -o -name \*.cpp -o -name \*.cc \) -type f | grep -v -e '/\.'`
$XGETTEXT $LIST -o $podir/koffice.pot

