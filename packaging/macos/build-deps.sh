cmake --build . --config RelWithDebInfo --target ext_boost
cmake --build . --config RelWithDebInfo --target ext_eigen3
cmake --build . --config RelWithDebInfo --target ext_exiv2
cmake --build . --config RelWithDebInfo --target ext_fftw3
cmake --build . --config RelWithDebInfo --target ext_ilmbase
cmake --build . --config RelWithDebInfo --target ext_jpeg
cmake --build . --config RelWithDebInfo --target ext_lcms2
cmake --build . --config RelWithDebInfo --target ext_ocio
cmake --build . --config RelWithDebInfo --target ext_openexr
install_name_tool -add_rpath /Users/boud/dev/i/lib /Users/boud/dev/b/ext_openexr/ext_openexr-prefix/src/ext_openexr-build/IlmImf/./b44ExpLogTable
install_name_tool -add_rpath /Users/boud/dev/i/lib /Users/boud/dev/b/ext_openexr/ext_openexr-prefix/src/ext_openexr-build/IlmImf/./dwaLookups
cmake --build . --config RelWithDebInfo --target ext_openexr
cmake --build . --config RelWithDebInfo --target ext_png
cmake --build . --config RelWithDebInfo --target ext_tiff
cmake --build . --config RelWithDebInfo --target ext_vc
cmake --build . --config RelWithDebInfo --target ext_libraw
cmake --build . --config RelWithDebInfo --target ext_giflib
cmake --build . --config RelWithDebInfo --target ext_kwindowsystem
cmake --build . --config RelWithDebInfo --target ext_gsl
