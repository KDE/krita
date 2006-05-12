#! /bin/sh
$EXTRACTRC `find . -name \*.ui` >> rc.cpp 
$EXTRACTRC `find . -name \*.rc` >> rc.cpp
perl extracti18n.pl > i18ndata
$XGETTEXT i18ndata rc.cpp `find . -name \*.cc -o -name \*.h` ui/kis_aboutdata.h -o $podir/krita.pot
rm -f i18ndata

