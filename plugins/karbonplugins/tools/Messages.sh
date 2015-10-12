#! /bin/sh
source ../../../calligra_xgettext.sh

calligra_xgettext krita_karbontools.pot `find . -name \*.cpp -o -name \*.h`
