#export MACOSX_DEPLOYMENT_TARGET=10.7
#export SDKROOT=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.7.sdk
cmake ../krita/3rdparty/  \
    -DCMAKE_INSTALL_PREFIX=/Users/boud/dev/i \
    -DEXTERNALS_DOWNLOAD_DIR=/Users/boud/dev/d  \
    -DINSTALL_ROOT=/Users/boud/dev/i 
 #   -DCMAKE_OSX_DEPLOYMENT_TARGET=10.7 \
 #   -CMAKE_OSX_SYSROOT=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.7.sdk
