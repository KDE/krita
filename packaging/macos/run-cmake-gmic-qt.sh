export MACOSX_DEPLOYMENT_TARGET=10.11
cmake ../gmic-qt \
    -DGMIC_QT_HOST=krita \
    -DCMAKE_INSTALL_PREFIX=/Users/boud/dev/i \
    -DCMAKE_PREFIX_PATH=/Users/boud/dev/deps \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=10.11 
