@echo off
:: This batch script is meant to prepare a Krita package folder to be zipped or
:: to be a base for the installer.
::
:: Just drop it next to the "i" install folder where the dependencies and Krita
:: binaries are.
::
:: Also copy filelist_bin_dll.txt and filelist_lib_dll.txt if you want more
:: fine-grained DLL dependencies copying.
::
:: You may want to review the following parameters.
::
:: Configuration parameters:
::
::   MINGW_GCC_BIN:    Path to the mingw-w64/bin dir
::   SEVENZIP_EXE:     Path to 7-Zip executable (either 7z.exe or 7zG.exe)
::   BUILDDIR_SRC:     Path to krita source dir (for package shortcut)
::   BUILDDIR_INSTALL: Path to INSTALL prefix of the build
::
:: Note that paths should only contain alphanumeric, _ and -, except for the
:: path to 7-Zip, which is fine if quoted.
::
set MINGW_GCC_BIN=C:\TDM-GCC-64\bin\
set SEVENZIP_EXE="C:\Program Files\7-Zip\7z.exe"
rem set BUILDDIR_SRC=%CD%\krita
set BUILDDIR_SRC=%CD%\krita
set BUILDDIR_INSTALL=%CD%\i

:: --------

set PATH=%MINGW_GCC_BIN%;%PATH%

echo Krita Windows packaging script
echo.
echo Configurations:
echo   MINGW_GCC_BIN:    %MINGW_GCC_BIN%
echo   SEVENZIP_EXE:     %SEVENZIP_EXE%
echo   BUILDDIR_SRC:     %BUILDDIR_SRC%
echo   BUILDDIR_INSTALL: %BUILDDIR_INSTALL%
echo.

if "%1" == "" (
	set PAUSE=pause
) else (
	set "PAUSE= "
)

:: Simple checking
if not exist %BUILDDIR_INSTALL% (
	echo ERROR: Cannot find the install folder!
	%PAUSE%
	exit /B 1
)
if not "%BUILDDIR_INSTALL: =%" == "%BUILDDIR_INSTALL%" (
	echo ERROR: Install path contains space, which will not work properly!
	%PAUSE%
	exit /B 1
)

:: Decide package name to use
if "%1" == "" (
	echo Please input a package name. It will be used for the output directory, package
	echo file name and as the top-level directory of the package.
	echo Alternatively, you can pass it as the first argument to this script.
	echo You should only use alphanumeric, _ and -
	echo.
	set /P pkg_name=Package name^>
	setlocal
	if "!pkg_name!" == "" (
		echo ERROR: You cannot choose an empty name!
		%PAUSE%
		exit /B 1
	)
	endlocal
) else (
	set pkg_name=%1
)
echo Package name is "%pkg_name%"

set pkg_root=%CD%\%pkg_name%
echo Packaging dir is %pkg_root%
echo.
if exist %pkg_root% (
	echo ERROR: Packaging dir already exists! Please remove or rename it first.
	%PAUSE%
	exit /B 1
)

echo.
echo Trying to guess GCC version...
g++ --version > NUL
if errorlevel 1 (
	echo ERROR: g++ is not working.
	%PAUSE%
	exit /B 1
)
for /f "delims=" %%a in ('g++ --version ^| find "g++"') do set GCC_VERSION_LINE=%%a
echo -- %GCC_VERSION_LINE%
if "%GCC_VERSION_LINE:tdm64=%" == "%GCC_VERSION_LINE%" (
	echo Compiler doesn't look like TDM64-GCC, assuming simple mingw-w64
	set IS_TDM=
) else (
	echo Compiler looks like TDM64-GCC
	set IS_TDM=1
)

echo.
echo Trying to guess target architecture...
objdump --version > NUL
if errorlevel 1 (
	echo ERROR: objdump is not working.
	%PAUSE%
	exit /B 1
)
for /f "delims=, tokens=1" %%a in ('objdump -f %BUILDDIR_INSTALL%\bin\krita.exe ^| find "architecture"') do set TARGET_ARCH_LINE=%%a
echo -- %TARGET_ARCH_LINE%
if "%TARGET_ARCH_LINE:x86-64=%" == "%TARGET_ARCH_LINE%" (
	echo Target looks like x86
	set IS_X64=
) else (
	echo Target looks like x64
	set IS_x64=1
)

echo.
echo Testing for objcopy...
objcopy --version > NUL
if errorlevel 1 (
	echo ERROR: objcopy is not working.
	%PAUSE%
	exit /B 1
)

echo.
echo Testing for strip...
strip --version > NUL
if errorlevel 1 (
	echo ERROR: strip is not working.
	%PAUSE%
	exit /B 1
)

echo.
echo Creating base directories...
mkdir %pkg_root% && ^
mkdir %pkg_root%\bin && ^
mkdir %pkg_root%\lib && ^
mkdir %pkg_root%\share
if errorlevel 1 (
	echo ERROR: Cannot create packaging dir tree!
	%PAUSE%
	exit /B 1
)

echo.
echo Copying GCC libraries...
if x%IS_TDM% == x (
	if x%is_x64% == x (
		:: mingw-w64 x86
		set "STDLIBS=gcc_s_dw2-1 gomp-1 stdc++-6 winpthread-1"
	) else (
		:: mingw-w64 x64
		set "STDLIBS=gcc_s_seh-1 gomp-1 stdc++-6 winpthread-1"
	)
) else (
	if x%is_x64% == x (
		:: TDM-GCC x86
		set "STDLIBS=gomp-1"
	) else (
		:: TDM-GCC x64
		set "STDLIBS=gomp_64-1"
	)
)
for %%L in (%STDLIBS%) do copy "%MINGW_GCC_BIN%\lib%%L.dll" %pkg_root%\bin

echo.
echo Copying files...
:: krita.exe
copy %BUILDDIR_INSTALL%\bin\krita.exe %pkg_root%\bin
:: DLLs from bin/
if exist filelist_bin_dll.txt (
	for /f %%F in (filelist_bin_dll.txt) do copy %BUILDDIR_INSTALL%\bin\%%F %pkg_root%\bin
) else (
	echo INFO: filelist_bin_dll.txt not found, copying all DLLs except Qt5 from bin/
	setlocal enableextensions enabledelayedexpansion
	for /f "delims=" %%F in ('dir /b "%BUILDDIR_INSTALL%\bin\*.dll"') do (
		set file=%%F
		set file=!file:~0,3!
		if not x!file! == xQt5 copy %BUILDDIR_INSTALL%\bin\%%F %pkg_root%\bin
	)
	endlocal
)
:: symsrv.yes for Dr. Mingw
copy %BUILDDIR_INSTALL%\bin\symsrv.yes %pkg_root%\bin
:: DLLs from lib/
if exist filelist_lib_dll.txt (
	for /f %%F in (filelist_lib_dll.txt) do copy %BUILDDIR_INSTALL%\lib\%%F %pkg_root%\bin
) else (
	echo INFO: filelist_lib_dll.txt not found, copying all DLLs from lib/
	copy %BUILDDIR_INSTALL%\lib\*.dll %pkg_root%\bin
)
:: Boost, there might be more than one leftover but we can't really do much
copy %BUILDDIR_INSTALL%\bin\libboost_system-*.dll %pkg_root%\bin
:: KF5 plugins may be placed at different locations depending on how Qt is built
xcopy /S /Y /I %BUILDDIR_INSTALL%\lib\plugins\imageformats %pkg_root%\bin\imageformats
xcopy /S /Y /I %BUILDDIR_INSTALL%\plugins\imageformats %pkg_root%\bin\imageformats
xcopy /S /Y /I %BUILDDIR_INSTALL%\lib\plugins\kf5 %pkg_root%\bin\kf5
xcopy /S /Y /I %BUILDDIR_INSTALL%\plugins\kf5 %pkg_root%\bin\kf5
:: Qt Translations
:: it seems that windeployqt does these, but only *some* of these???
mkdir %pkg_root%\bin\translations
setlocal enableextensions enabledelayedexpansion
for /f "delims=" %%F in ('dir /b "%BUILDDIR_INSTALL%\translations\qt_*.qm"') do (
	:: Exclude qt_help_*.qm
	set temp=%%F
	set temp2=!temp:_help=!
	if x!temp2! == x!temp! copy %BUILDDIR_INSTALL%\translations\!temp! %pkg_root%\bin\translations\!temp!
)
endlocal
:: Krita plugins
xcopy /Y %BUILDDIR_INSTALL%\lib\kritaplugins\*.dll %pkg_root%\lib\kritaplugins\

:: Share
xcopy /Y /S /I %BUILDDIR_INSTALL%\share\color %pkg_root%\share\color
xcopy /Y /S /I %BUILDDIR_INSTALL%\share\color-schemes %pkg_root%\share\color-schemes
xcopy /Y /S /I %BUILDDIR_INSTALL%\share\icons %pkg_root%\share\icons
xcopy /Y /S /I %BUILDDIR_INSTALL%\share\kf5 %pkg_root%\share\kf5
xcopy /Y /S /I %BUILDDIR_INSTALL%\share\krita %pkg_root%\share\krita
xcopy /Y /S /I %BUILDDIR_INSTALL%\share\kritaplugins %pkg_root%\share\kritaplugins
xcopy /Y /S /I %BUILDDIR_INSTALL%\share\mime %pkg_root%\share\mime
:: Not useful on Windows it seems
rem xcopy /Y /S /I %BUILDDIR_INSTALL%\share\appdata %pkg_root%\share\appdata
rem xcopy /Y /S /I %BUILDDIR_INSTALL%\share\applications %pkg_root%\share\applications
rem xcopy /Y /S /I %BUILDDIR_INSTALL%\share\doc %pkg_root%\share\doc
rem xcopy /Y /S /I %BUILDDIR_INSTALL%\share\kservices5 %pkg_root%\share\kservices5
rem xcopy /Y /S /I %BUILDDIR_INSTALL%\share\man %pkg_root%\share\man
rem xcopy /Y /S /I %BUILDDIR_INSTALL%\share\ocio %pkg_root%\share\ocio

:: Copy locale to bin
xcopy /Y /S /I %BUILDDIR_INSTALL%\share\locale %pkg_root%\bin\locale

:: Copy shortcut link from source (can't create it dynamically)
copy %BUILDDIR_SRC%\packaging\windows\krita.lnk %pkg_root%

:: windeployqt
%BUILDDIR_INSTALL%\bin\windeployqt.exe --release -concurrent -network -printsupport -svg -xml -multimedia %pkg_root%\bin\krita.exe

:: Copy embedded Python
xcopy /Y /S /I %BUILDDIR_INSTALL%\python %pkg_root%\python

:: For chopping relative path
:: 512 should be enough
:: n+2 to also account for a trailing backslash
setlocal enableextensions enabledelayedexpansion
for /L %%n in (1 1 512) do if "!pkg_root:~%%n,1!" neq "" set /a "pkg_root_len_plus_one=%%n+2"
endlocal & set pkg_root_len_plus_one=%pkg_root_len_plus_one%

echo.
setlocal enableextensions enabledelayedexpansion
:: Remove Python cache files
for /d /r "%pkg_root%" %%F in (__pycache__\) do (
	if EXIST "%%F" (
		set relpath=%%F
		set relpath=!relpath:~%pkg_root_len_plus_one%!
		echo Deleting Python cache !relpath!
		rmdir /S /Q "%%F"
	)
)
endlocal

echo.
echo Splitting debug info from binaries...
call :split-debug "%pkg_root%\bin\krita.exe" bin\krita.exe
setlocal enableextensions enabledelayedexpansion
:: Find all DLLs
for /r "%pkg_root%" %%F in (*.dll) do (
	set relpath=%%F
	set relpath=!relpath:~%pkg_root_len_plus_one%!
	call :split-debug "%%F" !relpath!
)
endlocal
setlocal enableextensions enabledelayedexpansion
:: Find all Python native modules
for /r "%pkg_root%\share\krita\pykrita\" %%F in (*.pyd) do (
	set relpath=%%F
	set relpath=!relpath:~%pkg_root_len_plus_one%!
	call :split-debug "%%F" !relpath!
)
endlocal

echo.
echo Packaging debug info...
:: (note that the top-level package dir is not included)
%SEVENZIP_EXE% a -tzip %pkg_name%-dbg.zip -r %pkg_root%\*.debug
echo --------

echo.
echo Packaging stripped binaries...
%SEVENZIP_EXE% a -tzip %pkg_name%.zip %pkg_root%\ -xr!.debug
echo --------

echo.
echo.
echo Krita packaged as %pkg_name%.zip
if exist %pkg_name%-dbg.zip echo Debug info packaged as %pkg_name%-dbg.zip
echo Packaging dir is %pkg_root%
echo NOTE: Do not create installer with packaging dir. Extract from
echo           %pkg_name%.zip instead,
echo       and do _not_ run krita inside the extracted directory because it will
echo       create extra unnecessary files.
echo.
echo Please remember to actually test the package before releasing it.
echo.
%PAUSE%

exit /b

:split-debug
echo Splitting debug info of %2
objcopy --only-keep-debug %~1 %~1.debug
if ERRORLEVEL 1 exit /b %ERRORLEVEL%
:: If the debug file is small enough then consider there being no debug info.
:: Discard these files since they somehow make gdb crash.
call :getfilesize %~1.debug
if /i %getfilesize_retval% LEQ 2048 (
	echo Discarding %2.debug
	del %~1.debug
	exit /b 0
)
if not exist %~dp1.debug mkdir %~dp1.debug
move %~1.debug %~dp1.debug\ > NUL
strip %~1
:: Add debuglink
:: FIXME: There is a problem with gdb that cause it to output this warning
:: FIXME: "warning: section .gnu_debuglink not found in xxx.debug"
:: FIXME: I tried adding a link to itself but this kills drmingw :(
objcopy --add-gnu-debuglink="%~dp1.debug\%~nx1.debug" %~1
exit /b %ERRORLEVEL%

:getfilesize
set getfilesize_retval=%~z1
goto :eof

:relpath_dirpath
call :relpath_dirpath_internal "" "%~1"
goto :eof

:relpath_dirpath_internal
for /f "tokens=1* delims=\" %%a in ("%~2") do (
	:: If part 2 is empty, it means part 1 is probably the file name
	if x%%b==x (
		set relpath_dirpath_retval=%~1
	) else (
		call :relpath_dirpath_internal "%~1%%a\" %%b
	)
)
goto :eof
