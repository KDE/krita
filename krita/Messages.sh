#! /bin/sh
$EXTRACTRC `find . -name \*.ui` >> rc.cpp 
$EXTRACTRC `find . -name \*.rc` >> rc.cpp
$EXTRACTATTR --attr=collection,text --attr=collection,comment --attr=script,text --attr=script,comment plugins/extensions/scripting/scripts/*.rc >> rc.cpp || exit 12
$EXTRACTATTR --attr=info,name plugins/colorspaces/ctlcs/ctlcolorspaces/*.ctlcs >> rc.cpp || exit 12
perl extracti18n.pl > i18ndata
# ignore sdk/templates since it contains templates for use a future plugins, none of the strings there will ever be seen by any user
$XGETTEXT i18ndata rc.cpp `find . -name \*.cc -o -name \*.h  -o -name \*.cpp | grep -v './sdk/templates'` ui/kis_aboutdata.h -o $podir/krita.pot
rm -f i18ndata

