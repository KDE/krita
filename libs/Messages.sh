#! /bin/sh
source ../krita_xgettext.sh

LIST=`find . \( -name \*.ui -o -name \*.rc \) -type f -print | grep -v -e '/\.'`
$EXTRACTRC $LIST >> rc.cpp
LIST=`find . \( -name \*.cpp -o -name \*.cc -o -name \*.h \) -type f -print | grep -v '/tests/' | grep -v -e '/\.'`
krita_xgettext kritalibs.pot $LIST
rm -f rc.cpp
