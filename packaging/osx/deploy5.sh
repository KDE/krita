set -e # stop on errors
set -x # be verbose 
/Users/boud/master/fixrpath.sh
rm -rf ~/master/krita.app/
rm -rf ~/master/krita.dmg
cp -r ~/master/i/bin/krita.app ~/master
cp -r ~/master/i/share/* ~/master/krita.app/Contents/Resources
cd  ~/master/krita.app/Contents
ln -s Resources share
mkdir -p  ~/master/krita.app/Contents/Library/QuickLook
cp -r  ~/master/i/lib/plugins/kritaquicklook.qlgenerator ~/master/krita.app/Contents/Library/QuickLook

~/master/deps/bin/macdeployqt ~/master/krita.app \
    -verbose=0 \
    -executable=/Users/boud/master/krita.app/Contents/MacOS/krita \
    -extra-plugins=/Users/boud/master/i/lib/kritaplugins/ \
    -extra-plugins=/Users/boud/master/i/lib/plugins/ \
    -extra-plugins=/Users/boud/master/i/plugins/ \
    -extra-plugins=/Users/boud/master/deps/plugins


#install_name_tool -delete_rpath  @loader_path/../../../../lib ~/master/krita.app/Contents/MacOS/krita

