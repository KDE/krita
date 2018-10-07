set -e # stop on errors
set -x # be verbose 
/Users/boud/dev/fixrpath.sh
rm -rf ~/dev/krita.app/
rm -rf ~/dev/krita.dmg
cp -r ~/dev/i/bin/krita.app ~/dev
cp -r ~/dev/i/share/* ~/dev/krita.app/Contents/Resources
cp -r ~/dev/deps/translations ~/dev/krita.app/Contents/Resources
cd  ~/dev/krita.app/Contents
ln -s Resources share
mkdir -p  ~/dev/krita.app/Contents/Library/QuickLook
cp -r  ~/dev/i/lib/plugins/kritaquicklook.qlgenerator ~/dev/krita.app/Contents/Library/QuickLook

~/dev/deps/bin/macdeployqt ~/dev/krita.app \
    -verbose=0 \
    -executable=/Users/boud/dev/krita.app/Contents/MacOS/krita \
    -qmldir=/Users/boud/dev/deps/qml \
    -extra-plugins=/Users/boud/dev/i/lib/kritaplugins/ \
    -extra-plugins=/Users/boud/dev/i/lib/plugins/ \
    -extra-plugins=/Users/boud/dev/i/plugins/ \
    -extra-plugins=/Users/boud/dev/deps/plugins

cp -r /Users/boud/dev/i/lib/qml Resources
cp -r /Users/boud/dev/deps/lib/python3.5 Frameworks/
cp -r /Users/boud/dev/i/lib/krita-python-libs Frameworks/
# XXX: fix rpath for krita.so
cp -r /Users/boud/dev/deps/sip Frameworks/

    
#install_name_tool -delete_rpath  @loader_path/../../../../lib ~/dev/krita.app/Contents/MacOS/krita

