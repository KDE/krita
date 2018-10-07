export QMAKE_MACOSX_DEPLOYMENT_TARGET=10.11
export MACOSX_DEPLOYMENT_TARGET=10.11
export SDKROOT=/Applications/Xcode.app/Contents/developer/Platforms/MacOSX.platform/developer/SDKs/MacOSX10.13.sdk

# Be verbose
set -x
# Exit on error
set -e 
/usr/bin/make ext_zlib
/usr/bin/make ext_giflib
/usr/bin/make ext_png
/usr/bin/make ext_lcms2
/usr/bin/make ext_jpeg
/usr/bin/make ext_openssl
/usr/bin/make ext_qt

export PATH=/Applications/CMake.app/Contents/bin/:/Users/boud/bin:/Users/boud/dev/deps/bin:/Users/boud/dev/deps/lib:/Users/boud/dev/i/bin:/Users/boud/i/lib:$PATH
export LIBRARY_PATH=/Users/boud/dev/deps/lib:/Users/boud/dev/i/lib:$LIBRARY_PATH
export C_INCLUDE_PATH=/Users/boud/dev/deps/include:/Users/boud/dev/i/include:$C_INCLUDE_PATH
export CPLUS_INCLUDE_PATH=/Users/boud/dev/deps/include:/Users/boud/dev/i/include:$CPLUS_INCLUDE_PATH
#export KRITA_PLUGIN_PATH=/Users/boud/dev/i/
export PKG_CONFIG_PATH=/Users/boud/dev/deps/lib/pkgconfig:/Users/boud/dev/deps/share/pkgconfig:$PKG_CONFIG_PATH

/usr/bin/make ext_python
/usr/bin/make ext_boost
/usr/bin/make ext_eigen3
/usr/bin/make ext_exiv2
/usr/bin/make ext_fftw3
/usr/bin/make ext_ilmbase
/usr/bin/make ext_ocio
/usr/bin/make ext_openexr
install_name_tool -add_rpath /Users/boud/dev/deps/lib /Users/boud/dev/b/ext_openexr/ext_openexr-prefix/src/ext_openexr-build/IlmImf/./b44ExpLogTable
install_name_tool -add_rpath /Users/boud/dev/deps/lib /Users/boud/dev/b/ext_openexr/ext_openexr-prefix/src/ext_openexr-build/IlmImf/./dwaLookups
/usr/bin/make ext_openexr
/usr/bin/make ext_tiff
/usr/bin/make ext_vc
/usr/bin/make ext_libraw
/usr/bin/make ext_gettext
/usr/bin/make ext_kcrash
/usr/bin/make ext_gsl
/usr/bin/make ext_freetype
/usr/bin/make ext_fontconfig
install_name_tool -add_rpath /Users/boud/dev/deps/lib ./ext_fontconfig/ext_fontconfig-prefix/src/ext_fontconfig-build/fc-cache/.libs/fc-cache 
/usr/bin/make ext_fontconfig
/usr/bin/make ext_poppler

export DEPS_INSTALL_PREFIX=/Users/boud/dev/deps
export PYTHONPATH=$DEPS_INSTALL_PREFIX/sip:$DEPS_INSTALL_PREFIX/lib/python3.5/site-packages:$DEPS_INSTALL_PREFIX/lib/python3.5
export PYTHONHOME=$DEPS_INSTALL_PREFIX

/usr/bin/make ext_sip
/usr/bin/make ext_pyqt

