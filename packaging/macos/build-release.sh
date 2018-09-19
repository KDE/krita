export MACOSX_DEPLOYMENT_TARGET=10.11
cmake ../krita-4.0.3 \
    -DCMAKE_INSTALL_PREFIX=/Users/boud/dev/i-krita-4.0.3 \
    -DCMAKE_PREFIX_PATH=/Users/boud/dev/deps \
    -DDEFINE_NO_DEPRECATED=1 -DBUILD_TESTING=OFF \
    -DKDE4_BUILD_TESTS=OFF \
    -DPACKAGERS_BUILD=ON  \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DBUNDLE_INSTALL_DIR=$HOME/dev/i/bin  \
    -DFOUNDATION_BUILD=ON \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=10.11
make -j4 install
../deploy5.sh
