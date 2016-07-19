rm -rf ~/dev/krita.app/
rm -rf ~/dev/krita.dmg
cp -r ~/dev/i/bin/krita.app ~/dev
cp -r ~/dev/i/share ~/dev/krita.app/Contents/
mkdir -p ~/dev/krita.app/Contents/PlugIns/krita
install_name_tool -add_rpath $HOME/dev/i/lib ~/dev/krita.app/Contents/MacOS/krita
macdeployqt ~/dev/krita.app \
    -verbose=0 \
    -executable=$HOME/dev/krita.app/Contents/MacOS/krita \
    -extra-plugins=$HOME/dev/i/lib/krita/ \
    -extra-plugins=$HOME/dev/i/lib/plugins/ \
    -extra-plugins=$HOME/dev/i/plugins/ 
mv ~/dev/krita.app/Contents/PlugIns/*so ~/dev/krita.app/Contents/PlugIns/krita
install_name_tool -delete_rpath  @loader_path/../../../../lib ~/dev/krita.app/Contents/MacOS/krita
install_name_tool -delete_rpath  $HOME/dev/i/lib ~/dev/krita.app/Contents/MacOS/krita

