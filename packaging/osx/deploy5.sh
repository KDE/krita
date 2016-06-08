set -e # stop on errors
set -x # be verbise 

rm -rf ~/dev/krita.app/
rm -rf ~/dev/krita.dmg
cp -r ~/dev/i/bin/krita.app ~/dev
cp -r ~/dev/i/share ~/dev/krita.app/Contents/

mkdir -p ~/dev/krita.app/Contents/PlugIns/kritaplugins

install_name_tool -add_rpath /Users/boud/dev/i/lib ~/dev/krita.app/Contents/MacOS/krita

~/dev/i/bin/macdeployqt ~/dev/krita.app \
    -verbose=1 \
    -executable=/Users/boud/dev/krita.app/Contents/MacOS/krita \
    -extra-plugins=/Users/boud/dev/i/lib/kritaplugins/ \
    -extra-plugins=/Users/boud/dev/i/lib/plugins/ \
    -extra-plugins=/Users/boud/dev/i/plugins/ \
    -dmg

#install_name_tool -delete_rpath  @loader_path/../../../../lib ~/dev/krita.app/Contents/MacOS/krita

