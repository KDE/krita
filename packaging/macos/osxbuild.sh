#!/usr/bin/env sh

# Stop at any error
# For debug purposes
# set -e
# set -x

# osxbuild.sh automates building and installing of krita and krita dependencies
# for OSX, the script only needs you to set BUILDROOT environment to work
# properly.
#

# Run with no args for a short help about each command.

# builddeps: Attempts to build krita dependencies in the necessary order,
#     intermediate steps for creating symlinks and fixing rpath of some
#     packages midway is also managed. Order goes from top to bottom, to add
#     new steps just place them in the proper place.

# rebuilddeps: This re-runs all make and make install of dependencies 3rdparty
#     this was needed as deleting the entire install directory an rerunning build
#     step for dependencies does not install if they are already built. This step
#     forces installation. Have not tested it lately so it might not be needed anymore

# build: Runs cmake build and make step for krita sources. It always run cmake step, so
#     it might take a bit longer than a pure <make> on the source tree. The script tries
#     to set the make flag -jN to a proper N.

# install: Runs install step for krita sources.

# fixboost: Search for all libraries using boost and sets a proper @rpath for boost as by
#     default it fails to set a proper @rpath

# buildinstall: Runs build, install and fixboost steps.


if test -z $BUILDROOT; then
    echo "ERROR: BUILDROOT env not set, exiting!"
    echo "\t Must point to the root of the buildfiles as stated in 3rdparty Readme"
    exit
fi

echo "BUILDROOT set to ${BUILDROOT}"
export KIS_SRC_DIR=${BUILDROOT}/krita
export KIS_TBUILD_DIR=${BUILDROOT}/depbuild
export KIS_TDEPINSTALL_DIR=${BUILDROOT}/depinstall
export KIS_DOWN_DIR=${BUILDROOT}/down
export KIS_BUILD_DIR=${BUILDROOT}/kisbuild
export KIS_INSTALL_DIR=${BUILDROOT}/i

# flags for OSX environment
# We only support from 10.11 up
export MACOSX_DEPLOYMENT_TARGET=10.11
export QMAKE_MACOSX_DEPLOYMENT_TARGET=10.11

# Build time variables
if test -z $(which cmake); then
    echo "ERROR: cmake not found, exiting!"
    exit
fi

export PATH=${KIS_INSTALL_DIR}/bin:$PATH
export C_INCLUDE_PATH=${KIS_INSTALL_DIR}/include:${C_INCLUDE_PATH}
export CPLUS_INCLUDE_PATH=${KIS_INSTALL_DIR}/include:${CPLUS_INCLUDE_PATH}
export LIBRARY_PATH=${KIS_INSTALL_DIR}/lib:${LIBRARY_PATH}
# export CPPFLAGS=-I${KIS_INSTALL_DIR}/include
# export LDFLAGS=-L${KIS_INSTALL_DIR}/lib

export PYTHONHOME=${KIS_INSTALL_DIR}
# export PYTHONPATH=${KIS_INSTALL_DIR}/sip:${KIS_INSTALL_DIR}/lib/python3.5/site-packages:${KIS_INSTALL_DIR}/lib/python3.5

# This will make the debug output prettier
export KDE_COLOR_DEBUG=1
export QTEST_COLORED=1


# configure max core for make compile
((MAKE_THREADS=1))
if test ${OSTYPE} == "darwin*"; then
    ((MAKE_THREADS = $(sysctl -n hw.ncpu) - 1))
fi

check_dir_path () {
    echo "Checking if ${1} exists and is dir… \r"
    if test -d ${1}; then
        echo "OK"
        return 0
    elif test -e ${1}; then
        echo "ERROR: file ${1} exists but is not a directory!"
        return 1
    else
        echo "Creating ${1}"
        mkdir ${1}
    fi
    return 0
}
# builds dependencies for the first time
build_3rdparty () {
    echo "building in ${KIS_TBUILD_DIR}"

    check_dir_path ${KIS_TBUILD_DIR}
    check_dir_path ${KIS_DOWN_DIR}
    check_dir_path ${KIS_INSTALL_DIR}

    cd ${KIS_TBUILD_DIR}

    cmake ${KIS_SRC_DIR}/3rdparty/ \
        -DCMAKE_INSTALL_PREFIX=${KIS_INSTALL_DIR} \
        -DEXTERNALS_DOWNLOAD_DIR=${KIS_DOWN_DIR} \
        -DINSTALL_ROOT=${KIS_INSTALL_DIR}

        # -DCPPFLAGS=-I${KIS_INSTALL_DIR}/include \
        # -DLDFLAGS=-L${KIS_INSTALL_DIR}/lib

    echo "finished 3rdparty build setup"
    # make preinstall
    echo "finished make step"
    
    cmake_3rdparty () {
        for package in ${@:1:${#@}}; do
            echo "Building ${package}"
            cmake --build . --config RelWithDebInfo --target ${package}
        done
    }
    
    if ! test -z ${1}; then
        echo "Starting build of ${1} $(test -n ${1})"
        cmake_3rdparty ${1}
        exit
    fi

    # build 3rdparty tools
    # The order must not be changed!
    cmake_3rdparty \
        ext_gettext \
        ext_openssl \
        ext_qt \
        ext_zlib \
        ext_boost \
        ext_eigen3 \
        ext_exiv2 \
        ext_fftw3 \
        ext_ilmbase \
        ext_jpeg \
        ext_lcms2 \
        ext_ocio
        
    # open exr will fail the first time is called
    # rpath needs to be fixed an build rerun
    if cmake_3rdparty ext_openexr; then
        echo "open_exr built success!"
    else
        if install_name_tool -add_rpath ${KIS_INSTALL_DIR}/lib ${KIS_TBUILD_DIR}/ext_openexr/ext_openexr-prefix/src/ext_openexr-build/IlmImf/./b44ExpLogTable; then
        echo "Added rpath for b44ExpLogTable"
        fi
        if install_name_tool -add_rpath ${KIS_INSTALL_DIR}/lib ${KIS_TBUILD_DIR}/ext_openexr/ext_openexr-prefix/src/ext_openexr-build/IlmImf/./dwaLookups; then
            echo "Added rpath for dwaLookups"
        fi
        echo "Rerunning ext_openexr build"
        cmake_3rdparty ext_openexr
    fi

    cmake_3rdparty \
        ext_png \
        ext_tiff \
        ext_gsl \
        ext_vc \
        ext_libraw \
        ext_giflib

    # for python
    # for sip PYQT_SIP_DIR_OVERRIDE=/usr/local/share/sip/qt5
    cmake_3rdparty \
        ext_python \
        ext_sip \
        ext_pyqt

    # on osx qmake is not correctly set to qmake-qt5 for some reason
    ln -s -f ${KIS_INSTALL_DIR}/bin/qmake ${KIS_INSTALL_DIR}/bin/qmake-qt5

    cmake_3rdparty \
        ext_frameworks \
        ext_extra_cmake_modules \
        ext_karchive \
        ext_kconfig \
        ext_kwidgetsaddons \
        ext_kcompletion \
        ext_kcoreaddons \
        ext_kguiaddons \
        ext_ki18n \
        ext_kitemmodels \
        ext_kitemviews \
        ext_kimageformats \
        ext_kwindowsystem \
        ext_quazip
}

# Recall cmake for all 3rd party packages
# make is only on target first run
# subsequent runs only call make install
rebuild_3rdparty () {
    echo "starting rebuild of 3rdparty packages"
    build_install_ext() {
        for pkg in ${@:1:${#@}}; do
            {
                cd ${KIS_TBUILD_DIR}/${pkg}/${pkg}-prefix/src/${pkg}-build
            } || {
                cd ${KIS_TBUILD_DIR}/${pkg}/${pkg}-prefix/src/${pkg}
            }
            echo "Installing ${pkg} files..."
            if test ${pkg} = "ext_boost"; then
                echo "Copying boost files..."
                cp bin.v2/libs/system/build/darwin-4.2.1/release/threading-multi/libboost_system.dylib ${KIS_INSTALL_DIR}/lib/
                cp bin.v2/libs/system/build/darwin-4.2.1/release/link-static/threading-multi/libboost_system.a ${KIS_INSTALL_DIR}/lib/
                # copy boost into i/include :: 
                cp -r boost ${KIS_INSTALL_DIR}/include
            else
                make -j${MAKE_THREADS}
                make install
            fi
            cd ${KIS_TBUILD_DIR}
        done
    }

    build_install_frameworks() {
        for pkg in \
                ext_extra_cmake_modules \
                ext_karchive \
                ext_kconfig \
                ext_kwidgetsaddons \
                ext_kcompletion \
                ext_kcoreaddons \
                ext_kguiaddons \
                ext_ki18n \
                ext_kitemmodels \
                ext_kitemviews \
                ext_kimageformats \
                ext_kwindowsystem \
                ; do
            cd ${KIS_TBUILD_DIR}/ext_frameworks/${pkg}-prefix/src/${pkg}-build
            echo "Installing ${pkg} files..."
            make install ${pkg}
        done
    }

    if ! test -n ${1}; then
        build_install_ext ${1}
        exit
    fi

    build_install_ext \
        ext_gettext \
        ext_openssl \

    # qt builds inside qt_ext, make install must be run inside.
    build_install_ext   ext_qt
    build_install_ext   ext_zlib

    # ext_boost
    build_install_ext   ext_boost

    build_install_ext \
                        ext_eigen3 \
                        ext_exiv2 \
                        ext_fftw3 \
                        ext_ilmbase \
                        ext_jpeg \
                        ext_lcms2 \
                        ext_ocio \
                        ext_openexr \
                        ext_png \
                        ext_tiff \
                        ext_gsl \
                        ext_vc \
                        ext_libraw \
                        ext_giflib

    # for python
    build_install_ext   ext_python
    build_install_ext   ext_sip
    build_install_ext   ext_pyqt

    # force creation of symlink, useful if install dir was wiped out.
    ln -s -f ${KIS_INSTALL_DIR}/bin/qmake ${KIS_INSTALL_DIR}/bin/qmake-qt5

    build_install_frameworks
    
}

#not tested
set_krita_dirs() {
    if ! test -z ${1}; then
        KIS_BUILD_DIR=${BUILDROOT}/b_${1}
        KIS_INSTALL_DIR=${BUILDROOT}/i_${1}
        KIS_SRC_DIR=${BUILDROOT}/src_${1}
    fi
}
# build_krita
# run cmake krita
build_krita () {
    set_krita_dirs ${1}
    echo ${KIS_BUILD_DIR}
    echo ${KIS_INSTALL_DIR}
    check_dir_path ${KIS_BUILD_DIR}
    cd ${KIS_BUILD_DIR}

    cmake ${KIS_SRC_DIR} \
        -DBoost_INCLUDE_DIR=${KIS_INSTALL_DIR}/include \
        -DCMAKE_INSTALL_PREFIX=${KIS_INSTALL_DIR} \
        -DDEFINE_NO_DEPRECATED=1 \
        -DBUILD_TESTING=OFF \
        -DHIDE_SAFE_ASSERTS=ON \
        -DKDE_INSTALL_BUNDLEDIR=${KIS_INSTALL_DIR}/bin \
        -DPYQT_SIP_DIR_OVERRIDE=$KIS_INSTALL_DIR/share/sip/ \
        -DCMAKE_BUILD_TYPE=RelWithDebInfo \
        -DCMAKE_OSX_DEPLOYMENT_TARGET=10.11

    # copiling phase
    make -j${MAKE_THREADS}

    # compile integrations
    if test ${OSTYPE} == "darwin*"; then
        cd ${KIS_BUILD_DIR}/krita/integration/kritaquicklook
        make -j${MAKE_THREADS}
    fi
}

install_krita () {
    set_krita_dirs ${1}
    check_dir_path ${KIS_BUILD_DIR}
    cd ${KIS_BUILD_DIR}
    make install

    # compile integrations
    if test ${OSTYPE} == "darwin*"; then
        cd ${KIS_BUILD_DIR}/krita/integration/kritaquicklook
        make install
    fi
}

fix_boost_rpath () {
    set_krita_dirs ${1}
    # install_name_tool -add_rpath ${KIS_INSTALL_DIR}/lib $BUILDROOT/$KRITA_INSTALL/bin/krita.app/Contents/MacOS/gmic_krita_qt
    if $(install_name_tool -add_rpath ${KIS_INSTALL_DIR}/lib ${KIS_INSTALL_DIR}/bin/krita.app/Contents/MacOS/krita); then
        echo "Added rpath ${KIS_INSTALL_DIR}/lib to krita bin"
    fi
    # install_name_tool -add_rpath ${BUILDROOT}/deps/lib ${KIS_INSTALL_DIR}/bin/krita.app/Contents/MacOS/krita
    install_name_tool -change libboost_system.dylib @rpath/libboost_system.dylib ${KIS_INSTALL_DIR}/bin/krita.app/Contents/MacOS/krita

    FILES=$(find -L ${KIS_INSTALL_DIR} -name '*so' -o -name '*dylib')
    for FILE in $FILES; do
        if test -n "$(otool -L $FILE | grep boost)"; then
            echo "Fixing… $FILE"
            install_name_tool -change libboost_system.dylib @rpath/libboost_system.dylib $FILE
        fi
    done
}

print_usage () {
    echo "USAGE: osxbuild.sh <buildstep> [pkg]"
    echo "BUILDSTEPS:\t\t"
    echo "\n builddeps \t\t Run cmake step for 3rd party dependencies, optionally takes a [pkg] arg"
    echo "\n rebuilddeps \t\t Rerun make and make install step for 3rd party deps, optionally takes a [pkg] arg
    \t\t\t usefull for cleaning install directory and quickly reinstall all deps."
    echo "\n fixboost \t\t Fixes broken boost \@rpath on OSX"
    echo "\n build \t\t\t Builds krita"
    echo "\n install \t\t Installs krita"
    echo "\n buildinstall \t\t Build and Installs krita, running fixboost after installing"
    echo ""
}

if test ${#} -eq 0; then
    echo "ERROR: No option given!"
    print_usage
    exit 1
fi

if test ${1} = "builddeps"; then
    build_3rdparty ${2}

elif test ${1} = "rebuilddeps"; then
    rebuild_3rdparty ${2}

elif test ${1} = "fixboost"; then
    fix_boost_rpath

elif test ${1} = "build"; then
    build_krita ${2}

elif test ${1} = "install"; then
    install_krita ${2}

elif test ${1} = "buildinstall"; then
    build_krita ${2}
    install_krita ${2}
    fix_boost_rpath ${2}

else
    echo "Option ${1} not supported"
    print_usage
fi

# after finishig sometimes it complains about missing matching quotes.