#! /bin/sh
source ../../krita_xgettext.sh

krita_xgettext krita-dockers.pot `find . -name \*.cpp`
