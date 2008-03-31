#! /bin/sh
$EXTRACTRC `find . -name \*.ui` >> rc.cpp 
$EXTRACTRC `find . -name \*.rc` >> rc.cpp
perl extracti18n.pl > i18ndata
# ignore sdk/templates since it contains templates for use a future plugins, none of the strings there will ever be seen by any user
$XGETTEXT i18ndata rc.cpp `find . -name \*.cc -o -name \*.h | grep -v './sdk/templates'` ui/kis_aboutdata.h -o $podir/krita.pot
rm -f i18ndata

