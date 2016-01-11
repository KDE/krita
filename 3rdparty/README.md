= CMake external projects to build krita's dependencies on Linux, Windows or OSX =

If you need to build Krita's dependencies for the following reasons:

* you develop on Windows and aren't using Emerge
* you develop on OSX and aren't using Homebrew
* you want to build a generic, distro-agnostic version of Krita for Linux
* you develop on Linux, but some dependencies aren't available for your distribution

and you know what you're doing, you can use the following guide to build
the dependencies that Krita needs.

If you develop on Linux and your distribution has the dependencies available,

YOU DO NOT NEED THIS GUIDE AND YOU SHOULD STOP READING NOW

Otherwise you risk major confusion.

== Prerequisites ==

Note: on all operating systems the entire procedure is done in a terminal window.

1. git: https://git-scm.com/downloads. Make sure git is in your path
2. cmake: https://cmake.org/download/. Make sure cmake is in your path.
3. Make sure you have a compiler:
    * Linux: gcc, minimum version 4.8
    * OSX: clang, you need to install xcode for this
    * Windows: MSVC 2015 Community Edition: https://www.visualstudio.com/en-us/products/visual-studio-community-vs.aspx

== Setup your environment ==

== Prepare your directory layout ==

1. Make a toplevel build directory, say $HOME/dev or c:\dev. We'll refer to this directory as BUILDROOT. You can use a variable for this, on WINDOWS %BUILDROOT%, on OSX and Linux $BUILDROOT. 

2. Checkout krita in BUILDROOT
    cd BUILDROOT
    git clone git://anongit.kde.org/krita.git
3. Create the build directory
    mkdir BUILDROOT/b
4. Create the downloads directory
    mkdir BUILDROOT/d
5. Create the install directory
    mkdir BUILDROOT/i

== Qt ==

Install Qt. Either build from source or with the qt.io installer. Make sure qmake is in your path. You need qtbase, qtsvg, qttools, qtscript, qtdeclarative, qtgraphicaleffects, qttranslations. On Windows, you also need qtwinextras, on Linux qtx11extras.

When installing from source, you can use these example configure commands:
 
* Linux:

    ./configure \
        -skip qt3d \
        -skip qtactiveqt \
        -skip qtcanvas3d \
        -skip qtconnectivity \
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
        -skip qtwebkit \
        -skip qtwebkit-examples \
        -skip qtwebsockets \
        -skip qtxmlpatterns \
        -opensource -confirm-license -release \
        -no-qml-debug -no-mtdev -no-journald \
        -no-openssl -no-libproxy \
        -no-pulseaudio -no-alsa -no-nis \
        -no-cups -no-tslib -no-pch \
        -no-dbus  -no-gstreamer -no-system-proxies \
        -no-openssl -no-libproxy -no-pulseaudio \
        -qt-xcb  -xcb -qt-freetype -qt-harfbuzz \
        -qt-pcre -qt-xkbcommon-x11 -xcb-xlib  \
        -prefix BUILDROOT/i
    make -j8

* OSX

./configure \
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
    -nomake examples -nomake demos \
    -prefix $BUILDROOT/i

* Windows

    configure -skip qt3d -skip qtactiveqt -skip qtcanvas3d -skip qtconnectivity -skip qtdoc -skip qtenginio -skip qtgraphicaleffects -skip qtlocation -skip qtmultimedia -skip qtsensors -skip qtserialport -skip qtwayland -skip qtwebchannel -skip qtwebengine -skip qtwebsockets -skip qtxmlpatterns -opensource -confirm-license -release -no-qml-debug -no-mtdev -no-openssl -no-libproxy -no-nis -no-dbus  -no-system-proxies -no-libproxy -opengl desktop -prefix %BUILDROOT%\i

== Prepare the externals build ==

1. enter the BUILDROOT/b directory
2. run cmake:
    * Linux:
    cmake ../krita/3rdparty -DINSTALL_ROOT=BUILDROOT/i -DEXTERNALS_DOWNLOAD_DIR=BUILDROOT/d -DCMAKE_INSTALL_PREFIX=BUILDROOT/i
:
    * OSX:
    * Windows 32 bits:

    * Windows 64 bits:

Note that the cmake command needs to point to your buildroot like /dev/d, not c:\dev\d.

    set PATH=BUILDROOT\i\bin\;BUILDROOT\i\lib;%PATH%
    cmake ..\krita\3rdparty -DEXTERNALS_DOWNLOAD_DIR=/dev/d -DINSTALL_ROOT=/dev/i   -G "Visual Studio 14 Win64" 
    

3. build the packages:

With a judicious application of DEPENDS statements, it's possible to build it all in one go, but in my experience that fails anyway, so it's better to build the dependencies indepdendently.

On Windows:

    cmake --build . --config RelWithDebInfo --target ext_patch
    cmake --build . --config RelWithDebInfo --target ext_png2ico
    cmake --build . --config RelWithDebInfo --target ext_pthreads


On all operating systems:

    cmake --build . --config RelWithDebInfo --target ext_boost
    cmake --build . --config RelWithDebInfo --target ext_eigen3
    cmake --build . --config RelWithDebInfo --target ext_exiv2
    cmake --build . --config RelWithDebInfo --target ext_fftw3

Note for Windows:

fftw3 is still broken, don't know why. Copy the bin, lib and include folders from 

    \b\ext_fftw3\ext_fftw3-prefix\src\ext_fftw3

manuall to BUILDROOT\i

    cmake --build . --config RelWithDebInfo --target ext_ilmbase
    cmake --build . --config RelWithDebInfo --target ext_jpeg
    cmake --build . --config RelWithDebInfo --target ext_lcms2
    cmake --build . --config RelWithDebInfo --target ext_ocio
    cmake --build . --config RelWithDebInfo --target ext_openexr

Note for OSX:

On OSX, you need to first build openexr; that will fail; then you need to set the rpath
for the two utilities correctly, then try to build openexr again.

    install_name_tool -add_rpath $BUILD_ROOT/i/lib $BUILD_ROOT/b/ext_openexr/ext_openexr-prefix/src/ext_openexr-build/IlmImf/./b44ExpLogTable
    install_name_tool -add_rpath $BUILD_ROOT/i/lib $BUILD_ROOT/b/ext_openexr/ext_openexr-prefix/src/ext_openexr-build/IlmImf/./dwaLookups

Note for Windows:

With MSVC 2015, the boost library has a name that later on makes the libraries unfindable, so you need to copy them.

    cd BUILDROOT\i\lib
    copy boost_system-vc-mt-1_55.dll boost_system140-mt-1_55.ddl
    copy boost_system-vc-mt-1_55.lib boost_system140-mt-1_55.lib


On All operatting systems:

    cmake --build . --config RelWithDebInfo --target ext_png
    cmake --build . --config RelWithDebInfo --target ext_tiff
    cmake --build . --config RelWithDebInfo --target ext_vc
    cmake --build . --config RelWithDebInfo --target ext_libraw
    cmake --build . --config RelWithDebInfo --target ext_openjpeg

On Windows and OSX

    cmake --build . --config RelWithDebInfo --target ext_kwindowsystem

On Windows

(Note: skip this for now if you're using msvc 2015, poppler isn't compatible
with that compiler yet)

    cmake --build . --config RelWithDebInfo --target ext_freetype
    cmake --build . --config RelWithDebInfo --target ext_poppler

On Linux

    cmake --build . --config RelWithDebInfo --target ext_kcrash


Note: poppler should be buildable on Linux as well with a home-built freetype
and fontconfig, but I don't know how to make fontconfig find freetype, and on
Linux, fontconfig is needed for poppler. Poppler is needed for PDF import.

Note 2: libcurl still isn't available.

== Build Krita ==
 
1. Make a krita build directory: 
    mkdir BUILDROOT/build
2. Enter the BUILDROOT/build
3. Run 

On Windows

    cmake ..\krita -G"Visual Studio 14 Win64" -DBoost_DEBUG=OFF -DBOOST_INCLUDEDIR=c:\dev\i\include -DBOOST_DEBUG=ON -DBOOST_ROOT=c:\dev\i -DBOOST_LIBRARYDIR=c:\dev\i\lib -DCMAKE_INSTALL_PREFIX=c:\dev\i -DCMAKE_PREFIX_PATH=c:\dev\i -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_TESTING=OFF -DKDE4_BUILD_TESTS=OFF -DHAVE_MEMORY_LEAK_TRACKER=OFF -DPACKAGERS_BUILD=ON -Wno-dev -DDEFINE_NO_DEPRECATED=1...\

On Linux

    cmake ../krita -DCMAKE_INSTALL_PREFIX=BUILDROOT/i -DDEFINE_NO_DEPRECATED=1 -DPACKAGERS_BUILD=ON -DBUILD_TESTING=OFF -DKDE4_BUILD_TESTS=OFF

On OSX

    cmake ../krita -DCMAKE_INSTALL_PREFIX=/Users/boud/dev/i -DDEFINE_NO_DEPRECATED=1 -DBUILD_TESTING=OFF -DKDE4_BUILD_TESTS=OFF -DPACKAGERS_BUILD=ON  -DBUNDLE_INSTALL_DIR=$HOME/dev/i/bin


4. Run 

On Linux and OSX
    make
    make install

On Windows
    cmake --build . --config RelWithDebInfo --target INSTALL

6. Run krita:

On Linux

    BUILDROOT/i/bin/krita

On Windows

    BUILDROOT\i\bin\krita.exe

On OSX

    BUILDROOT/i/bin/krita.app/Contents/MacOS/krita

