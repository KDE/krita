#! /bin/sh
source ../../calligra_xgettext.sh

calligra_xgettext kritacolorspaces.pot `find . -name \*.cpp -o -name \*.h`
