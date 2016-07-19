#! /bin/sh
source krita_xgettext.sh

$EXTRACTRC `find . -name \*.ui | grep -v '/tests/'` >> rc.cpp
RCFILES=`find . -name \*.xmlgui                                               \
	| grep -v plugins/extensions/metadataeditor/editors/dublincore.xmlgui \
	| grep -v plugins/extensions/metadataeditor/editors/exif.xmlgui       \
	| grep -v krita/sketch/KritaSketchWin.xmlgui                          \
	| grep -v krita/gemini/KritaGeminiWin.xmlgui
         `
$EXTRACTRC $RCFILES >> rc.cpp

ACTIONFILES=`find . -name \*.action`
./action_i18n.pl --context=action $ACTIONFILES >> rc.cpp

# extracti18n.pl extracts additional data from brushes, palettes etc.
perl extracti18n.pl > i18ndata

# Ignore sdk/templates which contains templates for writing future plugins.
# Also ignore crashreporter, it has it's own catalog
# None of the placeholder strings inside will be seen by users.
krita_xgettext krita.pot i18ndata rc.cpp \
                  `find . -name \*.cc -o -name \*.h  -o -name \*.cpp | \
                  grep -v '/tests/' | grep -v './sdk/templates' | grep -v './krita/crashreporter/'`

# Clean up
rm -f i18ndata rc.cpp
