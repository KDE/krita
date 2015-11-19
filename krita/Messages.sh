#! /bin/sh
source ../krita_xgettext.sh

$EXTRACTRC `find . -name \*.ui | grep -v '/tests/'` >> rc.cpp
RCFILES=`find . -name \*.rc                                                   \
	| grep -v plugins/extensions/metadataeditor/editors/dublincore.rc     \
	| grep -v plugins/extensions/metadataeditor/editors/exif.rc           \
	| grep -v sketch/KritaSketchWin.rc                                    \
	| grep -v gemini/KritaGeminiWin.rc
         `
$EXTRACTRC $RCFILES >> rc.cpp

ACTIONFILES=`find . -name \*.action`
./action_i18n.pl --context=action $ACTIONFILES >> rc.cpp

# extracti18n.pl extracts additional data from brushes, palettes etc.
perl extracti18n.pl > i18ndata

# Ignore sdk/templates which contains templates for writing future plugins.
# None of the placeholder strings inside will be seen by users.
krita_xgettext krita.pot i18ndata rc.cpp \
                  `find . -name \*.cc -o -name \*.h  -o -name \*.cpp | grep -v '/tests/' | grep -v './sdk/templates'`

# Clean up
rm -f i18ndata rc.cpp
