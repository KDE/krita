#export MACOSX_DEPLOYMENT_TARGET=10.9
#export SDKROOT=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk

./configure  \
    -skip qt3d \
    -skip qtactiveqt \
    -skip qtcanvas3d \
    -skip qtconnectivity \
    -skip qtdeclarative \
    -skip qtdoc \
    -skip qtenginio \
    -skip qtgraphicaleffects \
    -skip qtlocation \
    -skip qtmultimedia \
    -skip qtsensors \
    -skip qtserialport \
    -skip qtwayland \
    -skip qtwebchannel \
    -skip qtwebengine \
    -skip qtwebsockets \
    -skip qtxmlpatterns \
    -opensource -confirm-license -release \
    -no-qml-debug -no-mtdev -no-journald \
    -no-openssl -no-libproxy \
    -no-pulseaudio -no-alsa -no-nis \
    -no-cups -no-tslib -no-pch \
    -no-dbus  -no-gstreamer -no-system-proxies \
    -no-openssl -no-libproxy -no-pulseaudio \
    -nomake examples \
    -prefix /Users/boud/dev/i

#    -sdk macosx10.9 \
#    -platform macx-clang-64 \

