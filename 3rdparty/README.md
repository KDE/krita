= CMake external projects to build krita's dependencies on Linux, Windows or OSX =

If you need to build Krita's dependencies for the following reasons:

* you develop on Windows and aren't using krita\build-tools\windows\build.cmd or KDE's craft
* you develop on OSX and aren't using the scripts in krita\packaging\osx or Homebrew
* you want to build a generic, distro-agnostic version of Krita for Linux and aren't using the scripts in packaging\linux\appimage
* you develop on Linux, but some dependencies aren't available for your distribution and aren't using the scripts in packaging\linux\appimage

and you know what you're doing, you can use the following guide to build
the dependencies that Krita needs.

If you develop on Linux and your distribution has all dependencies available,

YOU DO NOT NEED THIS GUIDE AND YOU SHOULD STOP READING NOW

Otherwise you risk major confusion.

== Prerequisites ==

Note: on all operating systems the entire procedure is done in a terminal window.

1. git: https://git-scm.com/downloads. Make sure git is in your path
2. CMake 3.3.2 or later: https://cmake.org/download/. Make sure cmake is in your path.
    * CMake 3.9 does not build Krita properly at the moment, please use 3.8 or 3.10 instead.
3. Make sure you have a compiler:
    * Linux: gcc, minimum version 4.8
    * OSX: clang, you need to install xcode for this
    * Windows: mingw-w64 7.3 (by mingw-builds): https://sourceforge.net/projects/mingw-w64/
            Install mingw to something like c:\mingw; not to a folder with a space in its path.
            For threading, select posix, for exceptions seh.
               Make sure mingw's bin folder is in your path. It might be a good
               idea to create a batch file which sets the path and start cmd.
               MSVC is *not* supported at the moment.

4. On Windows, you will also need Python 3.6.2 (technically any versions of 3.6 is fine, but it's not tested): https://www.python.org. Make sure to have that version of python.exe in your path. This version of Python will be used for two things: to configure Qt and to build the Python scripting module. Make sure the version you download is exactly python-3.6.2. Make sure that this version of Python comes first in your path. Do not set PYTHONHOME or PYTHONPATH.

Make sure that your Python will have the correct architecture for the version you are trying to build: https://www.python.org/downloads/windows/ 

5. On Windows, if you want to compile Qt with ANGLE support, you will need to install Windows 10 SDK and have the environment variable `WindowsSdkDir` set to it (typically `C:\Program Files (x86)\Windows Kits\10`)

== Setup your environment ==


== Prepare your directory layout ==

1. Make a toplevel build directory, say $HOME/dev or c:\dev. We'll refer to this directory as BUILDROOT. You can use a variable for this, on WINDOWS %BUILDROOT%, on OSX and Linux $BUILDROOT. You will have to replace a bare BUILDROOT with $BUILDROOT or %BUILDROOT% whenever you copy and paste a command, depending on your operating system.

2. Checkout krita in BUILDROOT
    cd BUILDROOT
    git clone git://anongit.kde.org/krita.git
3. Create the build directory
    mkdir BUILDROOT/b
4. Create the downloads directory
    mkdir BUILDROOT/d
5. Create the install directory
    mkdir BUILDROOT/i

== Prepare the externals build ==

1. enter the BUILDROOT/b directory

2. run cmake:

    * Linux:
    export PATH=$BUILDROOT/i/bin:$PATH
    export PYTHONHOME=$BUILDROOT/i (only if you want to build your own python)
    cmake ../krita/3rdparty \
        -DINSTALL_ROOT=$BUILDROOT/i \
        -DEXTERNALS_DOWNLOAD_DIR=$BUILDROOT/d \
        -DCMAKE_INSTALL_PREFIX=BUILDROOT/i

    * OSX:

    export PATH=$BUILDROOT/i/bin:$PATH
    export PYTHONHOME=$BUILDROOT/i (only if you want to build your own python)
    cmake ../krita/3rdparty/  \
        -DCMAKE_INSTALL_PREFIX=$BUILDROOT/i \
        -DEXTERNALS_DOWNLOAD_DIR=$BUILDROOT/d  \
        -DINSTALL_ROOT=$BUILDROOT/i


    * Windows 32-bit / 64-bit:

Note that the cmake command needs to point to your BUILDROOT like /dev/d, not c:\dev\d.

    set PATH=%BUILDROOT%\i\bin\;%BUILDROOT%\i\lib;%PATH%
    cmake ..\krita\3rdparty -DEXTERNALS_DOWNLOAD_DIR=/dev/d -DINSTALL_ROOT=/dev/i  -G "MinGW Makefiles"

- If you want to build Qt and some other dependencies with parallel jobs, add
  `-DSUBMAKE_JOBS=<n>` to this cmake command where <n> is the number of jobs to
  run (if your PC has 4 CPU cores, you might want to set it to 5). For other jobs,
  you might need to manually add a -- -j N option, where N is the number of jobs.
  
- If you don't have Windows 10 SDK and don't want to build Qt with ANGLE, add
  `-DQT_ENABLE_DYNAMIC_OPENGL=OFF` to the CMake command line args.

3. build the packages:

On Windows:

    cmake --build . --config RelWithDebInfo --target ext_patch
    cmake --build . --config RelWithDebInfo --target ext_png2ico
    
On OSX and Windows:

    cmake --build . --config RelWithDebInfo --target ext_gettext

On all operating systems:

    cmake --build . --config RelWithDebInfo --target ext_qt
    cmake --build . --config RelWithDebInfo --target ext_zlib
    cmake --build . --config RelWithDebInfo --target ext_boost

	Note about boost: check if the headers are installed into i/include/boost, but not into i/include/boost-1.61/boost

    cmake --build . --config RelWithDebInfo --target ext_eigen3
    cmake --build . --config RelWithDebInfo --target ext_exiv2
    cmake --build . --config RelWithDebInfo --target ext_fftw3
    cmake --build . --config RelWithDebInfo --target ext_ilmbase
    cmake --build . --config RelWithDebInfo --target ext_jpeg
    cmake --build . --config RelWithDebInfo --target ext_lcms2
    cmake --build . --config RelWithDebInfo --target ext_ocio
    cmake --build . --config RelWithDebInfo --target ext_openexr

OSX Note: You need to first build openexr; that will fail; then you need to set the rpath for the two utilities correctly, then try to build openexr again.

    install_name_tool -add_rpath $BUILD_ROOT/i/lib $BUILD_ROOT/b/ext_openexr/ext_openexr-prefix/src/ext_openexr-build/IlmImf/./b44ExpLogTable
    install_name_tool -add_rpath $BUILD_ROOT/i/lib $BUILD_ROOT/b/ext_openexr/ext_openexr-prefix/src/ext_openexr-build/IlmImf/./dwaLookups



On All operating systems

    cmake --build . --config RelWithDebInfo --target ext_png
    cmake --build . --config RelWithDebInfo --target ext_tiff
    cmake --build . --config RelWithDebInfo --target ext_gsl
    cmake --build . --config RelWithDebInfo --target ext_vc
    cmake --build . --config RelWithDebInfo --target ext_libraw
    cmake --build . --config RelWithDebInfo --target ext_giflib

On Linux (if you want to build your own SIP and PyQt instead of the system one)

    cmake --build . --config RelWithDebInfo --target ext_sip
    cmake --build . --config RelWithDebInfo --target ext_pyqt

On Windows

    cmake --build . --config RelWithDebInfo --target ext_freetype
    cmake --build . --config RelWithDebInfo --target ext_poppler

On Linux

    cmake --build . --config RelWithDebInfo --target ext_kcrash

On Windows (if you want to include DrMingw for dumping backtrace on crash)

    cmake --build . --config RelWithDebInfo --target ext_drmingw

On Windows (if you want to include Python scripting)

    cmake --build . --config RelWithDebInfo --target ext_python
    cmake --build . --config RelWithDebInfo --target ext_sip
    cmake --build . --config RelWithDebInfo --target ext_pyqt

On Windows and Linux (if you want to include gmic-qt)

    cmake --build . --config RelWithDebInfo --target ext_gmic

Linux Note: poppler should be buildable on Linux as well with a home-built freetype
and fontconfig, but I don't know how to make fontconfig find freetype, and on
Linux, fontconfig is needed for poppler. Poppler is needed for PDF import.

OSX Note: In order to build fontconfig on macOS, you need to have pkg-config installed.
You probably need homebrew for that... See http://macappstore.org/pkg-config/ .
archives from: files.kde.org/krita/build/dependencies:

On Windows and OSX
    cmake --build . --config RelWithDebInfo --target ext_kwindowsystem



== Build Krita ==

1. Make a krita build directory:
    mkdir BUILDROOT/build
2. Enter the BUILDROOT/build
3. Run

On Windows

Depending on what you want to use, run this command for MSBuild:

    cmake ..\krita -G "MinGW Makefiles" -DBoost_DEBUG=OFF -DBOOST_INCLUDEDIR=c:\dev\i\include -DBOOST_DEBUG=ON -DBOOST_ROOT=c:\dev\i -DBOOST_LIBRARYDIR=c:\dev\i\lib -DCMAKE_INSTALL_PREFIX=c:\dev\i -DCMAKE_PREFIX_PATH=c:\dev\i -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_TESTING=OFF -DKDE4_BUILD_TESTS=OFF -DHAVE_MEMORY_LEAK_TRACKER=OFF -Wno-dev -DDEFINE_NO_DEPRECATED=1

Or this to use jom (faster compiling, uses all cores, ships with QtCreator/pre-built Qt binaries):

    cmake ..\krita -G "MinGW Makefiles" -DBoost_DEBUG=OFF -DBOOST_INCLUDEDIR=c:\dev\i\include -DBOOST_DEBUG=ON -DBOOST_ROOT=c:\dev\i -DBOOST_LIBRARYDIR=c:\dev\i\lib -DCMAKE_INSTALL_PREFIX=c:\dev\i -DCMAKE_PREFIX_PATH=c:\dev\i -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_TESTING=OFF -DKDE4_BUILD_TESTS=OFF -DHAVE_MEMORY_LEAK_TRACKER=OFF -Wno-dev -DDEFINE_NO_DEPRECATED=1

On Linux

   cmake ../krita -DCMAKE_INSTALL_PREFIX=BUILDROOT/i -DDEFINE_NO_DEPRECATED=1 -DBUILD_TESTING=OFF -DKDE4_BUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=RelWithDebInfo

   # Troubleshooting: if you built your own SIP and CMake fails to find it, please set
   #                  the following environment variable to the SIP installation directory:
   export PYTHONPATH=$BUILDROOT/i/sip/

   # If you also have KIO installed in the system, don't forget to disable it by bassing to cmake:
   # cmake -DCMAKE_DISABLE_FIND_PACKAGE_KF5KIO=true .

On OSX

    cmake ../krita -DCMAKE_INSTALL_PREFIX=$BUILDROOT/i -DDEFINE_NO_DEPRECATED=1 -DBUILD_TESTING=OFF -DKDE4_BUILD_TESTS=OFF -DBUNDLE_INSTALL_DIR=$BUILDROOT/i/bin -DCMAKE_BUILD_TYPE=RelWithDebInfo


4. Run

On Linux and OSX

    make
    make install

On Windows (replace 4 with the number of jobs to run in parallel)

    cmake --build . --target install -- -j4

6. Run krita:

On Linux

    BUILDROOT/i/bin/krita

On Windows

    BUILDROOT\i\bin\krita.exe

On OSX

    BUILDROOT/i/bin/krita.app/Contents/MacOS/krita

== Packaging a Windows Build ==

If you want to create a stripped down version of Krita to distribute, after building everything just copy the package_2.cmd file from the "windows" folder inside krita root source folder to BUILDROOT and run it (most likely C:\dev\).

That script will copy the necessary files into the specified folder and leave out developer related files. After the script runs there will be two new ZIP files that contain a small portable version of Krita and a separate portable debug version.
