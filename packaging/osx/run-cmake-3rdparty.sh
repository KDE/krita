export MACOSX_DEPLOYMENT_TARGET=10.11
cmake ../krita/3rdparty/  \
    -DCMAKE_INSTALL_PREFIX=/Users/boud/dev/deps \
    -DINSTALL_ROOT=/Users/boud/dev/deps \
    -DEXTERNALS_DOWNLOAD_DIR=/Users/boud/Downloads  \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=10.11 
