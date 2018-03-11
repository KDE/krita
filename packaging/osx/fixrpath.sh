install_name_tool -add_rpath /Users/boud/dev/i/lib ~/dev/i/bin/krita.app/Contents/MacOS/gmic_krita_qt
install_name_tool -add_rpath /Users/boud/dev/i/lib ~/dev/i/bin/krita.app/Contents/MacOS/krita
install_name_tool -add_rpath /Users/boud/dev/deps/lib ~/dev/i/bin/krita.app/Contents/MacOS/krita
install_name_tool -change libboost_system.dylib @rpath/libboost_system.dylib /Users/boud/dev/i/bin/krita.app/Contents/MacOS/krita


FILES="$(find /Users/boud/dev/i/lib/ -name '*so' -o -name '*dylib')"
for FILE in $FILES ; do
    echo $FILE
    install_name_tool -change libboost_system.dylib @rpath/libboost_system.dylib $FILE
done
