#! /bin/sh
$EXTRACTRC `find . -name \*.ui | grep -v '/tests/'` >> rc.cpp
RCFILES=`find . -name \*.rc                                                   \
	| grep -v plugins/extensions/metadataeditor/editors/dublincore.rc     \
	| grep -v plugins/extensions/metadataeditor/editors/exif.rc
         `
$EXTRACTRC $RCFILES >> rc.cpp
perl extracti18n.pl > i18ndata
# ignore sdk/templates since it contains templates for use a future plugins, none of the strings there will ever be seen by any user
$XGETTEXT i18ndata rc.cpp `find . -name \*.cc -o -name \*.h  -o -name \*.cpp | grep -v '/tests/' | grep -v './sdk/templates'` ui/kis_aboutdata.h -o $podir/krita.pot
rm -f i18ndata

