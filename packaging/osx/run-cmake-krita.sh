#export MACOSX_DEPLOYMENT_TARGET=10.7
#export SDKROOT=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.7.sdk
cmake ../krita \
    -DCMAKE_INSTALL_PREFIX=/Users/boud/dev/i \
    -DDEFINE_NO_DEPRECATED=1 -DBUILD_TESTING=OFF \
    -DKDE4_BUILD_TESTS=OFF \
    -DPACKAGERS_BUILD=ON  \
    -DBUNDLE_INSTALL_DIR=$HOME/dev/i/bin 
