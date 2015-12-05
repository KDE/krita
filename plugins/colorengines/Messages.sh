#! /bin/sh
source ../../krita_xgettext.sh

krita_xgettext kritacolorspaces.pot `find . -name \*.cpp -o -name \*.h`
