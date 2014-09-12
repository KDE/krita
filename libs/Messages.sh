#! /bin/sh
source ../calligra_xgettext.sh

EXCLUDE="-path ./koproperty -o -path ./db/drivers/sqlite" # TODO: -o -path ./koreport"
LIST=`find . \( $EXCLUDE \) -prune -o \( -name \*.ui -o -name \*.rc \) -type f -print | grep -v -e '/\.'`
$EXTRACTRC $LIST >> rc.cpp
LIST=`find . \( $EXCLUDE \) -prune -o \( -name \*.cpp -o -name \*.cc -o -name \*.h \) -type f -print | grep -v '/tests/' | grep -v -e '/\.'`
calligra_xgettext calligra.pot $LIST
rm -f rc.cpp
