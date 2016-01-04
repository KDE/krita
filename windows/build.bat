rem
rem Follow the instructions in 3rdparty/README.md to setup Qt and the 
rem build dirs and checkout krita
rem 

set PATH=c:\dev\i\bin\;c:\dev\i\lib;%PATH%
cd c:\dev\b
cmake ..\krita\3rdparty -DEXTERNALS_DOWNLOAD_DIR=/dev/d -DINSTALL_ROOT=/dev/i   -G "Visual Studio 14 Win64" 
cmake --build . --config RelWithDebInfo --target ext_patch
cmake --build . --config RelWithDebInfo --target ext_png2ico
cmake --build . --config RelWithDebInfo --target ext_pthreads
cmake --build . --config RelWithDebInfo --target ext_boost
copy c:\dev\i\lib\boost_system-vc-mt-1_55.dll c:\dev\i\lib\boost_system-vc140-mt-1_55.dll
copy c:\dev\i\lib\boost_system-vc-mt-1_55.lib c:\dev\i\lib\boost_system-vc140-mt-1_55.lib
cmake --build . --config RelWithDebInfo --target ext_eigen3
cmake --build . --config RelWithDebInfo --target ext_fftw3
xcopy ext_fftw3\ext_fftw3-prefix\src\ext_fftw3\bin c:\dev\i
xcopy ext_fftw3\ext_fftw3-prefix\src\ext_fftw3\lib c:\dev\i
xcopy ext_fftw3\ext_fftw3-prefix\src\ext_fftw3\include c:\dev\i
cmake --build . --config RelWithDebInfo --target ext_ilmbase
cmake --build . --config RelWithDebInfo --target ext_jpeg
cmake --build . --config RelWithDebInfo --target ext_lcms2
cmake --build . --config RelWithDebInfo --target ext_png
cmake --build . --config RelWithDebInfo --target ext_tiff
cmake --build . --config RelWithDebInfo --target ext_gsl
cmake --build . --config RelWithDebInfo --target ext_vc
cmake --build . --config RelWithDebInfo --target ext_libraw
cmake --build . --config RelWithDebInfo --target ext_openjpeg
cmake --build . --config RelWithDebInfo --target ext_freetype
cmake --build . --config RelWithDebInfo --target ext_ocio
cmake --build . --config RelWithDebInfo --target ext_openexr
cmake --build . --config RelWithDebInfo --target ext_exiv2
cmake --build . --config RelWithDebInfo --target ext_kwindowsystem

REM cmake --build . --config RelWithDebInfo --target ext_poppler
cd c:\dev\build
cmake ..\krita -G"Visual Studio 14 Win64" -DBoost_DEBUG=OFF -DBOOST_INCLUDEDIR=c:\dev\i\include -DBOOST_DEBUG=ON -DBOOST_ROOT=c:\dev\i -DBOOST_LIBRARYDIR=c:\dev\i\lib -DCMAKE_INSTALL_PREFIX=c:\dev\i -DCMAKE_PREFIX_PATH=c:\dev\i -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_TESTING=OFF -DKDE4_BUILD_TESTS=OFF -DHAVE_MEMORY_LEAK_TRACKER=OFF -DPACKAGERS_BUILD=ON -Wno-dev -DDEFINE_NO_DEPRECATED=1

