#! /bin/sh
source ../../../krita_xgettext.sh

krita_xgettext kritafiltereffects.pot `find . -name \*.cpp -o -name \*.h`
