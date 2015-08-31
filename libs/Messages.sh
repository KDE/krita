#! /bin/sh
source ../calligra_xgettext.sh

LIST=`find . \( -name \*.ui -o -name \*.rc \) -type f -print | grep -v -e '/\.'`
$EXTRACTRC $LIST >> rc.cpp
LIST=`find . \( -name \*.cpp -o -name \*.cc -o -name \*.h \) -type f -print | grep -v '/tests/' | grep -v -e '/\.'`
calligra_xgettext calligra.pot $LIST
rm -f rc.cpp
