#! /bin/sh
source ../../calligra_xgettext.sh

calligra_xgettext kocolorspaces.pot `find . -name \*.cpp -o -name \*.h`
