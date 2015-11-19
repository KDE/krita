#! /bin/sh
source ../../../krita_xgettext.sh

krita_xgettext krita_karbontools.pot `find . -name \*.cpp -o -name \*.h`
