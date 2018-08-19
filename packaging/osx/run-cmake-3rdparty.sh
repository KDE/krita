export MACOSX_DEPLOYMENT_TARGET=10.11
export QMAKE_MACOSX_DEPLOYMENT_TARGET=10.11
export MACOSX_DEPLOYMENT_TARGET=10.11
export SDKROOT=/Applications/Xcode.app/Contents/developer/Platforms/MacOSX.platform/developer/SDKs/MacOSX10.11.sdk

/Applications/CMake.app/Contents/bin/cmake ../krita/3rdparty/  \
    -DCMAKE_INSTALL_PREFIX=/Users/boud/dev/deps \
    -DINSTALL_ROOT=/Users/boud/dev/deps \
    -DEXTERNALS_DOWNLOAD_DIR=/Users/boud/Downloads  \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=10.11 
