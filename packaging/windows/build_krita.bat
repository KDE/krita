set DLLTOOL_EXE=c:\TDM-GCC-64\x86_64-w64-mingw32\bin\dlltool.exe
set MINGW_GCC_BIN=c:\TDM-GCC-64\bin\
set BUILDROOT=c:\dev
set BUILDDIR_INSTALL=%BUILDROOT%\i
set PATH=c:\dev\i\bin;c:\dev\i\lib;c:\python35;%MINGW_GCC_BIN%;%PATH%
cd c:\dev\build2

set /P pkg_root=Insert krita source location: 

if [%pkg_root%] == [] (
echo You entered an empty name!
pause
exit /B
)

cmake ..\%pkg_root% -G "MinGW Makefiles" -DBoost_DEBUG=OFF -DBOOST_INCLUDEDIR=c:\dev\i\include -DBOOST_DEBUG=ON -DBOOST_ROOT=c:\dev\i -DBOOST_LIBRARYDIR=c:\dev\i\lib -DCMAKE_INSTALL_PREFIX=c:\dev\i -DCMAKE_PREFIX_PATH=c:\dev\i -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_TESTING=OFF -DKDE4_BUILD_TESTS=OFF -DHAVE_MEMORY_LEAK_TRACKER=OFF -DPACKAGERS_BUILD=ON -Wno-dev -DDEFINE_NO_DEPRECATED=1 -DFOUNDATION_BUILD=1

mingw32-make -j4 install

cd ..
