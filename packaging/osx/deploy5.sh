set -e # stop on errors
set -x # be verbose 
/Users/boud/dev/bin/fixrpath.sh
rm -rf ~/dev/krita.app/
rm -rf ~/dev/krita.dmg
cp -r ~/dev/i/bin/krita.app ~/dev
cp -r ~/dev/i/share/* ~/dev/krita.app/Contents/Resources
cd  ~/dev/krita.app/Contents
ln -s Resources share
mkdir -p  ~/dev/krita.app/Contents/Library/QuickLook
cp ~/dev/i/lib/plugins/kritaquicklook.qlgenerator ~/dev/krita.app/Contents/Library/QuickLook

install_name_tool -add_rpath /Users/boud/dev/i/lib ~/dev/krita.app/Contents/MacOS/krita

~/dev/i/bin/macdeployqt ~/dev/krita.app \
    -verbose=1 \
    -executable=/Users/boud/dev/krita.app/Contents/MacOS/krita \
    -extra-plugins=/Users/boud/dev/i/lib/kritaplugins/ \
    -extra-plugins=/Users/boud/dev/i/lib/plugins/ \
    -extra-plugins=/Users/boud/dev/i/plugins/ 

#install_name_tool -delete_rpath  @loader_path/../../../../lib ~/dev/krita.app/Contents/MacOS/krita

