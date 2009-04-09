#! /bin/sh
LIST=`find . \( -name \*.ui -o -name \*.rc \) -type f | grep -v -e '/\.'`
$EXTRACTRC $LIST >> rc.cpp
LIST=`find . \( -name \*.cpp -o -name \*.cc -o -name \*.h \) -type f | grep -v -e '/\.'`
$XGETTEXT $LIST -o $podir/koffice.pot

