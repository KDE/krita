
# Be verbose
set -x
cmake --build . --config RelWithDebInfo --target ext_zlib
cmake --build . --config RelWithDebInfo --target ext_giflib
cmake --build . --config RelWithDebInfo --target ext_png
cmake --build . --config RelWithDebInfo --target ext_lcms2
cmake --build . --config RelWithDebInfo --target ext_jpeg
cmake --build . --config RelWithDebInfo --target ext_qt
cmake --build . --config RelWithDebInfo --target ext_boost
cmake --build . --config RelWithDebInfo --target ext_eigen3
cmake --build . --config RelWithDebInfo --target ext_exiv2
cmake --build . --config RelWithDebInfo --target ext_fftw3
cmake --build . --config RelWithDebInfo --target ext_ilmbase
cmake --build . --config RelWithDebInfo --target ext_ocio
cmake --build . --config RelWithDebInfo --target ext_openexr
install_name_tool -add_rpath /Users/boud/dev/deps/lib /Users/boud/dev/b/ext_openexr/ext_openexr-prefix/src/ext_openexr-build/IlmImf/./b44ExpLogTable
install_name_tool -add_rpath /Users/boud/dev/deps/lib /Users/boud/dev/b/ext_openexr/ext_openexr-prefix/src/ext_openexr-build/IlmImf/./dwaLookups
cmake --build . --config RelWithDebInfo --target ext_openexr
cmake --build . --config RelWithDebInfo --target ext_tiff
cmake --build . --config RelWithDebInfo --target ext_vc
cmake --build . --config RelWithDebInfo --target ext_libraw
cmake --build . --config RelWithDebInfo --target ext_gettext
cmake --build . --config RelWithDebInfo --target ext_kcrash
cmake --build . --config RelWithDebInfo --target ext_gsl
cmake --build . --config RelWithDebInfo --target ext_freetype
cmake --build . --config RelWithDebInfo --target ext_fontconfig
install_name_tool -add_rpath /Users/boud/dev/deps/lib ./ext_fontconfig/ext_fontconfig-prefix/src/ext_fontconfig-build/fc-cache/.libs/fc-cache 
cmake --build . --config RelWithDebInfo --target ext_fontconfig
cmake --build . --config RelWithDebInfo --target ext_poppler
cmake --build . --config RelWithDebInfo --target ext_python
cmake --build . --config RelWithDebInfo --target ext_sip
cmake --build . --config RelWithDebInfo --target ext_pyqt

