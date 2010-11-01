#! /bin/sh
EXCLUDE="-path ./koproperty -o -path ./koreports"
LIST=`find . \( \( $EXCLUDE \) -prune -o -name \*.ui -o -name \*.rc \) -type f | grep -v -e '/\.'`
$EXTRACTRC $LIST >> rc.cpp
LIST=`find . \( \( $EXCLUDE \) -prune -o -name \*.cpp -o -name \*.cc -o -name \*.h \) -type f | grep -v '/tests/' | grep -v -e '/\.'`
$XGETTEXT $LIST -o $podir/koffice.pot
