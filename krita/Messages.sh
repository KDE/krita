#! /bin/sh
source ../calligra_xgettext.sh

$EXTRACTRC `find . -name \*.ui | grep -v '/tests/'` >> rc.cpp
RCFILES=`find . -name \*.rc                                                   \
	| grep -v plugins/extensions/metadataeditor/editors/dublincore.rc     \
	| grep -v plugins/extensions/metadataeditor/editors/exif.rc           \
	| grep -v sketch/KritaSketchWin.rc                                    \
	| grep -v gemini/KritaGeminiWin.rc
         `
$EXTRACTRC $RCFILES >> rc.cpp
perl extracti18n.pl > i18ndata
# ignore sdk/templates since it contains templates for use a future plugins, none of the strings there will ever be seen by any user
calligra_xgettext krita.pot i18ndata rc.cpp `find . -name \*.cc -o -name \*.h  -o -name \*.cpp | grep -v '/tests/' | grep -v './sdk/templates'`
rm -f i18ndata rc.cpp
