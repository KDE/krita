#!/bin/sh

# Invoke the extractrc script on all .ui, .rc, and .kcfg files in the sources.
# The results are stored in a pseudo .cpp file to be picked up by xgettext.
lst=`find . -name \*.rc -o -name \*.ui -o -name \*.kcfg`
if [ -n "$lst" ] ; then
    $EXTRACTRC $lst >> rc.cpp
fi

# If your framework contains tips-of-the-day, call preparetips as well.
if [ -f "data/tips" ] ; then
    ( cd data && $PREPARETIPS > ../tips.cpp )
fi

# Extract strings from all source files.
# If your framework depends on KI18n, use $XGETTEXT. If it uses Qt translation
# system, use $EXTRACT_TR_STRINGS.
$XGETTEXT `find . -name \*.cpp -o -name \*.h -o -name \*.qml` -o $podir/kxmlgui5.pot
