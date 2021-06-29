@echo off
:: This batch script is meant to prepare a Krita package folder to be zipped or
:: to be a base for the installer.

:: --------

setlocal enabledelayedexpansion
goto begin


:: Subroutines

:find_on_path out_variable file_name
set %1=%~f$PATH:2
goto :EOF


:get_dir_path out_variable file_path
set %1=%~dp2
goto :EOF


:get_full_path out_variable file_path
setlocal
set FULL_PATH=%~f2
if not exist "%FULL_PATH%" (
    set FULL_PATH=
) else (
    if exist "%FULL_PATH%\" (
        set FULL_PATH=
    )
)
endlocal & set "%1=%FULL_PATH%"
goto :EOF


:get_full_path_dir out_variable file_path
setlocal
set FULL_PATH=%~dp2
if not exist "%FULL_PATH%" (
    set FULL_PATH=
)
endlocal & set "%1=%FULL_PATH%"
goto :EOF


:prompt_for_string out_variable prompt
set /p %1=%~2^>
goto :EOF


:prompt_for_positive_integer out_variable prompt
setlocal
call :prompt_for_string USER_INPUT "%~2"
if "%USER_INPUT%" == "" set USER_INPUT=0
set /a RESULT=%USER_INPUT%
if not %RESULT% GTR 0 (
    set RESULT=
)
endlocal & set "%1=%RESULT%"
goto :EOF


:prompt_for_file out_variable prompt
setlocal
:prompt_for_file__retry
call :prompt_for_string USER_INPUT "%~2"
if "%USER_INPUT%" == "" (
    endlocal
    set %1=
    goto :EOF
)
call :get_full_path RESULT "%USER_INPUT%"
if "%RESULT%" == "" (
    echo Input does not point to valid file!
    set USER_INPUT=
    goto prompt_for_file__retry
)
endlocal & set "%1=%RESULT%"
goto :EOF


:prompt_for_dir out_variable prompt
setlocal
:prompt_for_dir__retry
call :prompt_for_string USER_INPUT "%~2"
if "%USER_INPUT%" == "" (
    endlocal
    set %1=
    goto :EOF
)
call :get_full_path_dir RESULT "%USER_INPUT%\"
if "%RESULT%" == "" (
    echo Input does not point to valid dir!
    set USER_INPUT=
    goto prompt_for_dir__retry
)
endlocal & set "%1=%RESULT%"
goto :EOF


:usage
echo Usage:
echo %~n0 [--no-interactive --package-name ^<name^>] [ OPTIONS ... ]
echo.
echo Basic options:
echo --no-interactive                Run without interactive prompts
echo                                 When not specified, the script will prompt
echo                                 for some of the parameters.
echo --package-name ^<name^>           Specify the package name
echo.
echo Path options:
echo --src-dir ^<dir_path^>            Specify Krita source dir
echo                                 If unspecified, this will be determined from
echo                                 the script location
echo --deps-install-dir ^<dir_path^>   Specify deps install dir
echo --krita-install-dir ^<dir_path^>  Specify Krita install dir
echo.
echo Special options:
echo --pre-zip-hook ^<script_path^>    Specify a script to be called before
echo                                 packaging the zip archive, can be used to
echo                                 sign the binaries
echo.
goto :EOF
:usage_and_exit
call :usage
exit /b
:usage_and_fail
call :usage
exit /b 100


:: ----------------------------
:begin

echo Krita Windows packaging script
echo.


:: command-line args parsing
set ARG_NO_INTERACTIVE=
set ARG_PACKAGE_NAME=
set ARG_SRC_DIR=
set ARG_DEPS_INSTALL_DIR=
set ARG_KRITA_INSTALL_DIR=
set ARG_PRE_ZIP_HOOK=
:args_parsing_loop
set CURRENT_MATCHED=
if not "%1" == "" (
    if "%1" == "--no-interactive" (
        set ARG_NO_INTERACTIVE=1
        set CURRENT_MATCHED=1
    )
    if "%1" == "--package-name" (
        if not "%ARG_PACKAGE_NAME%" == "" (
            echo ERROR: Arg --package-name specified more than once 1>&2
            echo.
            goto usage_and_fail
        )
        if "%~2" == "" (
            echo ERROR: Arg --package-name is empty 1>&2
            echo.
            goto usage_and_fail
        )
        set "ARG_PACKAGE_NAME=%~2"
        shift /2
        set CURRENT_MATCHED=1
    )
    if "%1" == "--src-dir" (
        if not "%ARG_SRC_DIR%" == "" (
            echo ERROR: Arg --src-dir specified more than once 1>&2
            echo.
            goto usage_and_fail
        )
        if not exist "%~f2\" (
            echo ERROR: Arg --src-dir does not point to a directory 1>&2
            echo.
            goto usage_and_fail
        )
        call :get_dir_path ARG_SRC_DIR "%~f2\"
        shift /2
        set CURRENT_MATCHED=1
    )
    if "%1" == "--deps-install-dir" (
        if not "%ARG_DEPS_INSTALL_DIR%" == "" (
            echo ERROR: Arg --deps-install-dir specified more than once 1>&2
            echo.
            goto usage_and_fail
        )
        if "%~f2" == "" (
            echo ERROR: Arg --deps-install-dir does not point to a valid path 1>&2
            echo.
            goto usage_and_fail
        )
        call :get_dir_path ARG_DEPS_INSTALL_DIR "%~f2\"
        shift /2
        set CURRENT_MATCHED=1
    )
    if "%1" == "--krita-install-dir" (
        if not "%ARG_KRITA_INSTALL_DIR%" == "" (
            echo ERROR: Arg --krita-install-dir specified more than once 1>&2
            echo.
            goto usage_and_fail
        )
        if "%~f2" == "" (
            echo ERROR: Arg --krita-install-dir does not point to a valid path 1>&2
            echo.
            goto usage_and_fail
        )
        call :get_dir_path ARG_KRITA_INSTALL_DIR "%~f2\"
        shift /2
        set CURRENT_MATCHED=1
    )
    if "%1" == "--pre-zip-hook" (
        if not "%ARG_PRE_ZIP_HOOK%" == "" (
            echo ERROR: Arg --pre-zip-hook specified more than once 1>&2
            echo.
            goto usage_and_fail
        )
        if "%~f2" == "" (
            echo ERROR: Arg --pre-zip-hook does not point to a valid path 1>&2
            echo.
            goto usage_and_fail
        )
        call :get_full_path ARG_PRE_ZIP_HOOK "%~f2"
        if "!ARG_PRE_ZIP_HOOK!" == "" (
            echo ERROR: Arg --pre-zip-hook does not point to a valid file 1>&2
            echo.
            goto usage_and_fail
        )
        shift /2
        set CURRENT_MATCHED=1
    )
    if "%1" == "--help" (
        goto usage_and_exit
    )
    if not "!CURRENT_MATCHED!" == "1" (
        echo ERROR: Unknown option %1 1>&2
        echo.
        goto usage_and_fail
    )
    shift /1
    goto args_parsing_loop
)

if "%ARG_NO_INTERACTIVE%" == "1" (
    if "%ARG_PACKAGE_NAME%" == "" (
        echo ERROR: Required arg --package-name not specified! 1>&2
        echo.
        goto usage_and_fail
    )
)

if "%ARG_NO_INTERACTIVE%" == "1" (
    echo Non-interactive mode
) else (
    echo Interactive mode
    :: Trick to pause on exit
    call :real_begin
    pause
    exit /b !ERRORLEVEL!
)
:real_begin
echo.


if "%ARG_PACKAGE_NAME%" == "" (
    call :prompt_for_string ARG_PACKAGE_NAME "Provide package name"
    if "ARG_PACKAGE_NAME" == "" (
        echo ERROR: Package name not set! 1>&2
        exit /b 102
    )
)


:: Check environment config

if "%SEVENZIP_EXE%" == "" (
    call :find_on_path SEVENZIP_EXE 7z.exe
)
if "%SEVENZIP_EXE%" == "" (
    call :find_on_path SEVENZIP_EXE 7za.exe
)
if "!SEVENZIP_EXE!" == "" (
    set "SEVENZIP_EXE=%ProgramFiles%\7-Zip\7z.exe"
    if not exist "!SEVENZIP_EXE!" (
        set "SEVENZIP_EXE=%ProgramFiles(x86)%\7-Zip\7z.exe"
    )
    if not exist "!SEVENZIP_EXE!" (
        echo 7-Zip not found! 1>&2
        exit /b 102
    )
)
echo 7-Zip: %SEVENZIP_EXE%

if "%MINGW_BIN_DIR%" == "" (
    call :find_on_path MINGW_BIN_DIR_MAKE_EXE mingw32-make.exe
    if "!MINGW_BIN_DIR_MAKE_EXE!" == "" (
        if not "%ARG_NO_INTERACTIVE%" == "1" (
            call :prompt_for_file MINGW_BIN_DIR_MAKE_EXE "Provide path to mingw32-make.exe of mingw-w64"
        )
        if "!MINGW_BIN_DIR_MAKE_EXE!" == "" (
            echo ERROR: mingw-w64 not found! 1>&2
            exit /b 102
        )
        call :get_dir_path MINGW_BIN_DIR "!MINGW_BIN_DIR_MAKE_EXE!"
    ) else (
        call :get_dir_path MINGW_BIN_DIR "!MINGW_BIN_DIR_MAKE_EXE!"
        echo Found mingw on PATH: !MINGW_BIN_DIR!
        if not "%ARG_NO_INTERACTIVE%" == "1" (
            choice /c ny /n /m "Is this correct? [y/n] "
            if errorlevel 3 exit 255
            if not errorlevel 2 (
                call :prompt_for_file MINGW_BIN_DIR_MAKE_EXE "Provide path to mingw32-make.exe of mingw-w64"
                if "!MINGW_BIN_DIR_MAKE_EXE!" == "" (
                    echo ERROR: mingw-w64 not found! 1>&2
                    exit /b 102
                )
                call :get_dir_path MINGW_BIN_DIR "!MINGW_BIN_DIR_MAKE_EXE!"
            )
        )
    )
)
echo mingw-w64: %MINGW_BIN_DIR%

:: Windows SDK is needed for windeployqt to get d3dcompiler_xx.dll
if "%WindowsSdkDir%" == "" if not "%ProgramFiles(x86)%" == "" set "WindowsSdkDir=%ProgramFiles(x86)%\Windows Kits\10"
if "%WindowsSdkDir%" == "" set "WindowsSdkDir=%ProgramFiles(x86)%\Windows Kits\10"
if exist "%WindowsSdkDir%\" (
    pushd "%WindowsSdkDir%"
    if exist "bin\x64\fxc.exe" (
        set HAVE_FXC_EXE=1
    ) else (
        for /f "delims=" %%a in ('dir /a:d /b "bin\10.*"') do (
            if exist "bin\%%a\x64\fxc.exe" (
                set HAVE_FXC_EXE=1
            )
        )
    )
    popd
)
if not "%HAVE_FXC_EXE%" == "1" (
    set WindowsSdkDir=
    echo Windows SDK 10 with fxc.exe not found
    echo If Qt was built with ANGLE ^(dynamic OpenGL^) support, the package might not work properly on some systems!
) else echo Windows SDK 10 with fxc.exe found on %WindowsSdkDir%

if not "%ARG_SRC_DIR%" == "" (
    set "KRITA_SRC_DIR=%ARG_SRC_DIR%"
)
if "%KRITA_SRC_DIR%" == "" (
    :: Check whether this looks like to be in the source tree
	set "_temp=%~dp0"
	if "!_temp:~-19!" == "\packaging\windows\" (
        if exist "!_temp:~0,-19!\CMakeLists.txt" (
            if exist "!_temp:~0,-19!\3rdparty\CMakeLists.txt" (
                set "KRITA_SRC_DIR=!_temp:~0,-19!\"
                echo Script is running inside Krita src dir
            )
        )
    )
)
if "%KRITA_SRC_DIR%" == "" (
    if not "%ARG_NO_INTERACTIVE%" == "1" (
        call :prompt_for_dir KRITA_SRC_DIR "Provide path of Krita src dir"
    )
    if "!KRITA_SRC_DIR!" == "" (
        echo ERROR: Krita src dir not found! 1>&2
        exit /b 102
    )
)
echo Krita src: %KRITA_SRC_DIR%

if not "%ARG_DEPS_INSTALL_DIR%" == "" (
    set "DEPS_INSTALL_DIR=%ARG_DEPS_INSTALL_DIR%"
)
if "%DEPS_INSTALL_DIR%" == "" (
    set DEPS_INSTALL_DIR=%CD%\i_deps\
    echo Using default deps install dir: !DEPS_INSTALL_DIR!
    if not "%ARG_NO_INTERACTIVE%" == "1" (
        choice /c ny /n /m "Is this ok? [y/n] "
        if errorlevel 3 exit 255
        if not errorlevel 2 (
            call :prompt_for_dir DEPS_INSTALL_DIR "Provide path of deps install dir"
        )
    )
    if "!DEPS_INSTALL_DIR!" == "" (
        echo ERROR: Deps install dir not set! 1>&2
        exit /b 102
    )
)
echo Deps install dir: %DEPS_INSTALL_DIR%

if not "%ARG_KRITA_INSTALL_DIR%" == "" (
    set "KRITA_INSTALL_DIR=%ARG_KRITA_INSTALL_DIR%"
)
if "%KRITA_INSTALL_DIR%" == "" (
    set KRITA_INSTALL_DIR=%CD%\i\
    echo Using default Krita install dir: !KRITA_INSTALL_DIR!
    if not "%ARG_NO_INTERACTIVE%" == "1" (
        choice /c ny /n /m "Is this ok? [y/n] "
        if errorlevel 3 exit 255
        if not errorlevel 2 (
            call :prompt_for_dir KRITA_INSTALL_DIR "Provide path of Krita install dir"
        )
    )
    if "!KRITA_INSTALL_DIR!" == "" (
        echo ERROR: Krita install dir not set! 1>&2
        exit /b 102
    )
)
echo Krita install dir: %KRITA_INSTALL_DIR%


:: Simple checking
if not exist "%DEPS_INSTALL_DIR%\" (
	echo ERROR: Cannot find the deps install folder! 1>&2
	exit /B 1
)
if not exist "%KRITA_INSTALL_DIR%\" (
	echo ERROR: Cannot find the krita install folder! 1>&2
	exit /B 1
)
if not "%DEPS_INSTALL_DIR: =%" == "%DEPS_INSTALL_DIR%" (
	echo ERROR: Deps install path contains space, which will not work properly! 1>&2
	exit /B 1
)
if not "%KRITA_INSTALL_DIR: =%" == "%KRITA_INSTALL_DIR%" (
	echo ERROR: Krita install path contains space, which will not work properly! 1>&2
	exit /B 1
)

set pkg_name=%ARG_PACKAGE_NAME%
echo Package name is "%pkg_name%"

set pkg_root=%CD%\%pkg_name%
echo Packaging dir is %pkg_root%
echo.
if exist %pkg_root% (
	echo ERROR: Packaging dir already exists! Please remove or rename it first.
	exit /B 1
)
if exist %pkg_root%.zip (
	echo ERROR: Packaging zip already exists! Please remove or rename it first.
	exit /B 1
)
if exist %pkg_root%-dbg.zip (
	echo ERROR: Packaging debug zip already exists! Please remove or rename it first.
	exit /B 1
)

echo.


if not "%ARG_NO_INTERACTIVE%" == "1" (
    choice /c ny /n /m "Is the above ok? [y/n] "
    if errorlevel 3 exit 255
    if not errorlevel 2 (
        exit /b 1
    )
    echo.
)


:: Initialize clean PATH
set PATH=%SystemRoot%\system32;%SystemRoot%;%SystemRoot%\System32\Wbem;%SYSTEMROOT%\System32\WindowsPowerShell\v1.0\
set PATH=%MINGW_BIN_DIR%;%DEPS_INSTALL_DIR%\bin;%PATH%


echo.
echo Trying to guess GCC version...
g++ --version > NUL
if errorlevel 1 (
	echo ERROR: g++ is not working.
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
	exit /B 1
)
for /f "delims=, tokens=1" %%a in ('objdump -f %KRITA_INSTALL_DIR%\bin\krita.exe ^| find "architecture"') do set TARGET_ARCH_LINE=%%a
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
	exit /B 1
)

echo.
echo Testing for strip...
strip --version > NUL
if errorlevel 1 (
	echo ERROR: strip is not working.
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
for %%L in (%STDLIBS%) do copy "%MINGW_BIN_DIR%\lib%%L.dll" %pkg_root%\bin

echo.
echo Copying files...
:: krita.exe
copy %KRITA_INSTALL_DIR%\bin\krita.exe %pkg_root%\bin
copy %KRITA_INSTALL_DIR%\bin\krita.com %pkg_root%\bin
:: kritarunner.exe
copy %KRITA_INSTALL_DIR%\bin\kritarunner.exe %pkg_root%\bin
copy %KRITA_INSTALL_DIR%\bin\kritarunner.com %pkg_root%\bin

if exist %KRITA_INSTALL_DIR%\bin\FreehandStrokeBenchmark.exe (
    :: FreehandStrokeBenchmark.exe
    copy %KRITA_INSTALL_DIR%\bin\FreehandStrokeBenchmark.exe %pkg_root%\bin
    xcopy /S /Y /I %DEPS_INSTALL_DIR%\bin\data %pkg_root%\bin\data
)

:: qt.conf -- to specify the location to Qt translations
copy %KRITA_SRC_DIR%\packaging\windows\qt.conf %pkg_root%\bin
:: DLLs from bin/
echo INFO: Copying all DLLs except Qt5* from bin/
setlocal enableextensions enabledelayedexpansion
for /f "delims=" %%F in ('dir /b "%KRITA_INSTALL_DIR%\bin\*.dll"') do (
	set file=%%F
	set file=!file:~0,3!
	if not x!file! == xQt5 copy %KRITA_INSTALL_DIR%\bin\%%F %pkg_root%\bin
)
for /f "delims=" %%F in ('dir /b "%DEPS_INSTALL_DIR%\bin\*.dll"') do (
	set file=%%F
	set file=!file:~0,3!
	if not x!file! == xQt5 copy %DEPS_INSTALL_DIR%\bin\%%F %pkg_root%\bin
)
endlocal
:: symsrv.yes for Dr. Mingw
copy %DEPS_INSTALL_DIR%\bin\symsrv.yes %pkg_root%\bin
:: DLLs from lib/
echo INFO: Copying all DLLs from lib/ (deps)
copy %DEPS_INSTALL_DIR%\lib\*.dll %pkg_root%\bin
:: Boost, there might be more than one leftover but we can't really do much
copy %DEPS_INSTALL_DIR%\bin\libboost_system-*.dll %pkg_root%\bin
:: KF5 plugins may be placed at different locations depending on how Qt is built
xcopy /S /Y /I %DEPS_INSTALL_DIR%\lib\plugins\imageformats %pkg_root%\bin\imageformats
xcopy /S /Y /I %DEPS_INSTALL_DIR%\plugins\imageformats %pkg_root%\bin\imageformats
xcopy /S /Y /I %DEPS_INSTALL_DIR%\lib\plugins\kf5 %pkg_root%\bin\kf5
xcopy /S /Y /I %DEPS_INSTALL_DIR%\plugins\kf5 %pkg_root%\bin\kf5

:: Copy the sql drivers explicitly
xcopy /S /Y /I %DEPS_INSTALL_DIR%\plugins\sqldrivers %pkg_root%\bin\sqldrivers

:: Qt Translations
:: it seems that windeployqt does these, but only *some* of these???
mkdir %pkg_root%\bin\translations
setlocal enableextensions enabledelayedexpansion
for /f "delims=" %%F in ('dir /b "%DEPS_INSTALL_DIR%\translations\qt_*.qm"') do (
	:: Exclude qt_help_*.qm
	set temp=%%F
	set temp2=!temp:_help=!
	if x!temp2! == x!temp! copy %DEPS_INSTALL_DIR%\translations\!temp! %pkg_root%\bin\translations\!temp!
)
endlocal
:: Krita plugins
xcopy /Y %KRITA_INSTALL_DIR%\lib\kritaplugins\*.dll %pkg_root%\lib\kritaplugins\
xcopy /Y /S /I %DEPS_INSTALL_DIR%\lib\krita-python-libs %pkg_root%\lib\krita-python-libs
xcopy /Y /S /I %KRITA_INSTALL_DIR%\lib\krita-python-libs %pkg_root%\lib\krita-python-libs

:: Share
xcopy /Y /S /I %KRITA_INSTALL_DIR%\share\color %pkg_root%\share\color
xcopy /Y /S /I %KRITA_INSTALL_DIR%\share\color-schemes %pkg_root%\share\color-schemes
xcopy /Y /S /I %KRITA_INSTALL_DIR%\share\icons %pkg_root%\share\icons
xcopy /Y /S /I %KRITA_INSTALL_DIR%\share\krita %pkg_root%\share\krita
xcopy /Y /S /I %KRITA_INSTALL_DIR%\share\kritaplugins %pkg_root%\share\kritaplugins
xcopy /Y /S /I %DEPS_INSTALL_DIR%\share\kf5 %pkg_root%\share\kf5
xcopy /Y /S /I %DEPS_INSTALL_DIR%\share\mime %pkg_root%\share\mime
:: Python libs
xcopy /Y /S /I %DEPS_INSTALL_DIR%\share\krita\pykrita %pkg_root%\share\krita\pykrita
xcopy /Y /S /I %DEPS_INSTALL_DIR%\lib\site-packages %pkg_root%\lib\site-packages
:: Not useful on Windows it seems
rem xcopy /Y /S /I %KRITA_INSTALL_DIR%\share\appdata %pkg_root%\share\appdata
rem xcopy /Y /S /I %KRITA_INSTALL_DIR%\share\applications %pkg_root%\share\applications
rem xcopy /Y /S /I %DEPS_INSTALL_DIR%\share\doc %pkg_root%\share\doc
rem xcopy /Y /S /I %DEPS_INSTALL_DIR%\share\kservices5 %pkg_root%\share\kservices5
rem xcopy /Y /S /I %DEPS_INSTALL_DIR%\share\man %pkg_root%\share\man
rem xcopy /Y /S /I %DEPS_INSTALL_DIR%\share\ocio %pkg_root%\share\ocio

:: Copy locale to bin
xcopy /Y /S /I %KRITA_INSTALL_DIR%\share\locale %pkg_root%\bin\locale
xcopy /Y /S /I %DEPS_INSTALL_DIR%\share\locale %pkg_root%\bin\locale

:: Copy shortcut link from source (can't create it dynamically)
copy %KRITA_SRC_DIR%\packaging\windows\krita.lnk %pkg_root%
copy %KRITA_SRC_DIR%\packaging\windows\krita-minimal.lnk %pkg_root%
copy %KRITA_SRC_DIR%\packaging\windows\krita-animation.lnk %pkg_root%

set "QMLDIR_ARGS=--qmldir %DEPS_INSTALL_DIR%\qml"
if exist "%KRITA_INSTALL_DIR%\lib\qml" (
    xcopy /Y /S /I %KRITA_INSTALL_DIR%\lib\qml %pkg_root%\bin
    :: This doesn't really seem to do anything
    set "QMLDIR_ARGS=%QMLDIR_ARGS% --qmldir %KRITA_INSTALL_DIR%\lib\qml"
)

:: windeployqt
windeployqt.exe %QMLDIR_ARGS% --release -gui -core -concurrent -network -printsupport -svg -xml -sql -multimedia -qml -quick -quickwidgets %pkg_root%\bin\krita.exe %pkg_root%\bin\krita.dll
if errorlevel 1 (
	echo ERROR: WinDeployQt failed! 1>&2
	exit /B 1
)

:: ffmpeg
copy %DEPS_INSTALL_DIR%\bin\ffmpeg.exe %pkg_root%\bin
copy %DEPS_INSTALL_DIR%\bin\ffmpeg_LICENSE.txt %pkg_root%\bin
copy %DEPS_INSTALL_DIR%\bin\ffmpeg_README.txt %pkg_root%\bin


:: Copy embedded Python
xcopy /Y /S /I %DEPS_INSTALL_DIR%\python %pkg_root%\python
del /q %pkg_root%\python\python.exe %pkg_root%\python\pythonw.exe

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
call :split-debug "%pkg_root%\bin\krita.com" bin\krita.com
call :split-debug "%pkg_root%\bin\kritarunner.exe" bin\kritarunner.exe
call :split-debug "%pkg_root%\bin\kritarunner.com" bin\kritarunner.com
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
for /r "%pkg_root%\lib\krita-python-libs\" %%F in (*.pyd) do (
	set relpath=%%F
	set relpath=!relpath:~%pkg_root_len_plus_one%!
	call :split-debug "%%F" !relpath!
)
for /r "%pkg_root%\lib\site-packages\" %%F in (*.pyd) do (
	set relpath=%%F
	set relpath=!relpath:~%pkg_root_len_plus_one%!
	call :split-debug "%%F" !relpath!
)
endlocal

if not "%ARG_PRE_ZIP_HOOK%" == "" (
    echo Running pre-zip-hook...
    setlocal
    cmd /c ""%ARG_PRE_ZIP_HOOK%" "%pkg_root%\""
    if errorlevel 1 (
        echo ERROR: Got exit code !errorlevel! from pre-zip-hook! 1>&2
        exit /b 1
    )
    endlocal
)

echo.
echo Packaging stripped binaries...
"%SEVENZIP_EXE%" a -tzip %pkg_name%.zip %pkg_root%\ "-xr^!.debug"
echo --------

echo.
echo Packaging debug info...
:: (note that the top-level package dir is not included)
"%SEVENZIP_EXE%" a -tzip %pkg_name%-dbg.zip -r %pkg_root%\*.debug
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
strip --strip-debug %~1
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
