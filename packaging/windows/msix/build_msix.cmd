@echo off

setlocal enableextensions enabledelayedexpansion

if not defined OUTPUT_DIR set "OUTPUT_DIR=%CD%\out"
if not defined KRITA_DIR (
    if not defined KRITA_INSTALLER (
        echo ERROR: KRITA_DIR and KRITA_INSTALLER not specified, one of them must be set.
        exit /b 1
    )
)
rem  For binary factory, define both KRITA_DIR and KRITA_SHELLEX.
rem  Do not use KRITA_SHELLEX if building outside of binary factory, unless you
rem  don't mind this script modifying the contents of KRITA_DIR.
if defined KRITA_INSTALLER (
    if defined KRITA_SHELLEX (
        echo ERROR: KRITA_SHELLEX must not be set if using KRITA_INSTALLER.
        exit /b 1
    )
)

goto begin


:: Subroutines

:get_rel_path out_variable file_path base_path
setlocal enableextensions
set FULL_PATH=%~f2
set FULL_BASE=%~f3
for /L %%n in (1 1 512) do if "!FULL_BASE:~%%n,1!" neq "" set /a "FULL_BASE_len=%%n+1"
set /a "FULL_BASE_len_plus_one=%FULL_BASE_len%+1"
if not exist "%FULL_PATH%" (
    set REL_PATH=
) else (
    if not exist "%FULL_BASE%" (
        set REL_PATH=
    ) else (
        if "!FULL_PATH:~0,%FULL_BASE_len%!" == "%FULL_BASE%" (
            set REL_PATH=!FULL_PATH:~%FULL_BASE_len_plus_one%!
        ) else (
            set REl_PATH=
        )
    )
)
endlocal & set "%1=%REL_PATH%"
goto :EOF


:get_temp_file out_variable
setlocal enableextensions
set "uniqueFileName=%tmp%\bat~%RANDOM%.tmp"
if exist "%uniqueFileName%" call :get_temp_file uniqueFileName
endlocal & set "%1=%uniqueFileName%"
goto :EOF


:find_on_path out_variable file_name
set %1=%~f$PATH:2
goto :EOF


:: ----------------------------
:begin


echo *** Krita MSIX build script ***

if "%WindowsSdkDir%" == "" if not "%ProgramFiles(x86)%" == "" set "WindowsSdkDir=%ProgramFiles(x86)%\Windows Kits\10"
if "%WindowsSdkDir%" == "" set "WindowsSdkDir=%ProgramFiles(x86)%\Windows Kits\10"
if exist "%WindowsSdkDir%\" (
    pushd "%WindowsSdkDir%"
    for /f "delims=" %%a in ('dir /a:d /b "bin\10.*"') do (
        if exist "bin\%%a\x64\makeappx.exe" (
            set "MAKEPRI=%WindowsSdkDir%\bin\%%a\x64\makepri.exe"
        )
        if exist "bin\%%a\x64\makeappx.exe" (
            set "MAKEAPPX=%WindowsSdkDir%\bin\%%a\x64\makeappx.exe"
        )
        if exist "bin\%%a\x64\signtool.exe" (
            set "SIGNTOOL=%WindowsSdkDir%\bin\%%a\x64\signtool.exe"
        )
    )
    if "%MAKEPRI%" == "" if exist "bin\x64\makepri.exe" (
        set "MAKEPRI=%WindowsSdkDir%\bin\x64\makepri.exe"
    )
    if "%MAKEAPPX%" == "" if exist "bin\x64\makeappx.exe" (
        set "MAKEAPPX=%WindowsSdkDir%\bin\x64\makeappx.exe"
    )
    if "%SIGNTOOL%" == "" if exist "bin\x64\signtool.exe" (
        set "SIGNTOOL=%WindowsSdkDir%\bin\x64\signtool.exe"
    )
    popd
)
if "%MAKEPRI%" == "" (
    echo ERROR: makepri not found 1>&2
    exit /b 1
)
if "%MAKEAPPX%" == "" (
    echo ERROR: makeappx not found 1>&2
    exit /b 1
)
if "%SIGNTOOL%" == "" (
    echo ERROR: signtool not found 1>&2
    exit /b 1
)

set SCRIPT_DIR=%~dp0
mkdir "%OUTPUT_DIR%"


if defined KRITA_DIR goto skip_extract_installer

echo.
echo === Step 0: Extract files from installer

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

pushd %OUTPUT_DIR%
if errorlevel 1 (
    echo ERROR entering "%OUTPUT_DIR%" 1>&2
    exit /B 1
)

mkdir installer_content
cd installer_content
"%SEVENZIP_EXE%" x "%KRITA_INSTALLER%"
if errorlevel 1 (
    echo ERROR failed to extract installer "%KRITA_INSTALLER%" 1>&2
    exit /B 1
)
rmdir /s /q $PLUGINSDIR
del /q uninstall.exe.nsis uninstall.exe
set KRITA_DIR=%CD%

popd

echo === Step 0 done. ===


:skip_extract_installer

if not defined KRITA_SHELLEX goto skip_copy_shellex_files

echo.
echo === Step 0: Copy files for shell extension

pushd "%KRITA_DIR%"
if errorlevel 1 (
    echo ERROR entering "%KRITA_DIR%" 1>&2
    exit /B 1
)
mkdir shellex
if errorlevel 1 (
    echo ERROR mkdir shellex failed 1>&2
    exit /B 1
)
copy "%KRITA_SHELLEX%\krita.ico" shellex
if errorlevel 1 (
    echo ERROR copying krita.ico failed 1>&2
    exit /B 1
)
copy "%KRITA_SHELLEX%\kritafile.ico" shellex
if errorlevel 1 (
    echo ERROR copying kritafile.ico failed 1>&2
    exit /B 1
)
copy "%KRITA_SHELLEX%\kritashellex32.dll" shellex
if errorlevel 1 (
    echo ERROR copying kritashellex32.dll failed 1>&2
    exit /B 1
)
copy "%KRITA_SHELLEX%\kritashellex64.dll" shellex
if errorlevel 1 (
    echo ERROR copying kritashellex64.dll failed 1>&2
    exit /B 1
)
:: Optional files:
copy "%KRITA_SHELLEX%\kritashellex32.pdb" shellex
copy "%KRITA_SHELLEX%\kritashellex64.pdb" shellex

popd

echo === Step 0 done. ===


:skip_copy_shellex_files

rem Sanity checks:
if not exist "%KRITA_DIR%\bin\krita.exe" (
    echo ERROR: KRITA_DIR is set to "%KRITA_DIR%" but "%KRITA_DIR%\bin\krita.exe" does not exist! 1>&2
    exit /B 1
)
if not exist "%KRITA_DIR%\shellex\kritashellex64.dll" (
    echo ERROR: "%KRITA_DIR%\shellex\kritashellex64.dll" does not exist! 1>&2
    exit /B 1
)
if exist "%KRITA_DIR%\bin\.debug" (
    echo ERROR: Package dir seems to contain debug symbols [gcc/mingw].
    exit /B 1
)
if exist "%KRITA_DIR%\bin\*.pdb" (
    echo ERROR: Package dir seems to contain debug symbols [msvc].
    exit /B 1
)
if exist "%KRITA_DIR%\$PLUGINSDIR" (
    echo ERROR: You did not remove "$PLUGINSDIR".
    exit /B 1
)
if exist "%KRITA_DIR%\uninstall.exe.nsis" (
    echo ERROR: You did not remove "uninstall.exe.nsis".
    exit /B 1
)
if exist "%KRITA_DIR%\uninstall.exe*" (
    echo ERROR: You did not remove "uninstall.exe*".
    exit /B 1
)


echo.
echo === Step 1: Generate resources.pri ===

"%MAKEPRI%" new /pr "%SCRIPT_DIR%pkg" /mn "%SCRIPT_DIR%manifest.xml" /cf "%SCRIPT_DIR%priconfig.xml" /o /of "%OUTPUT_DIR%\resources.pri"
if errorlevel 1 (
    echo ERROR running makepri 1>&2
    exit /B 1
)

echo === Step 1 done. ===


echo.
echo === Step 2: Generate file mapping list ===

set "ASSETS_DIR=%SCRIPT_DIR%pkg\Assets"
set "MAPPING_OUT=%OUTPUT_DIR%\mapping.txt"


call :get_temp_file OUT_TEMP
echo Writing list to temporary file "%OUT_TEMP%"

echo [Files] > %OUT_TEMP%
echo "%SCRIPT_DIR%manifest.xml" "AppxManifest.xml" >> %OUT_TEMP%
echo "%OUTPUT_DIR%\resources.pri" "Resources.pri" >> %OUT_TEMP%

rem Krita application files:
pushd "%KRITA_DIR%"
for /r %%a in (*) do (
    call :get_rel_path rel "%%a" "%CD%"
    echo "%%a" "krita\!rel!" >> %OUT_TEMP%
)
popd

rem Assets:
pushd "%ASSETS_DIR%"
for /r %%a in (*) do (
    call :get_rel_path rel "%%a" "%CD%"
    echo "%%a" "Assets\!rel!" >> %OUT_TEMP%
)
popd

copy "%OUT_TEMP%" "%MAPPING_OUT%"
del "%OUT_TEMP%"

echo Written mapping file to "%MAPPING_OUT%"
echo === Step 2 done. ===


echo.
echo === Step 3: Make MSIX with makeappx.exe ===

goto step3_begin

:: ----------------------------
(this is a comment block...)

For reference, the MSIX Packaging tool uses the following command arguments:
    pack /v /o /l /nv /nfv /f "%UserProfile%\AppData\Local\Packages\Microsoft.MsixPackagingTool_8wekyb3d8bbwe\LocalState\DiagOutputDir\Logs\wox1ifkc.h0i.txt" /p "D:\dev\krita\msix\Krita-testing_4.3.0.0_x64__svcxxs8w6n55m.msix"

The arguments stands for:
    pack: Creates a package.
    /v: Enable verbose logging output to the console.
    /o: Overwrites the output file if it exists. If you don't specify this option or the /no option, the user is asked whether they want to overwrite the file.
    /l: Used for localized packages. The default validation trips on localized packages. This options disables only that specific validation, without requiring that all validation be disabled.
    /nv: Skips semantic validation. If you don't specify this option, the tool performs a full validation of the package.
    /nfv: ???
    /f <mapping file>: Specifies the mapping file.
    /p <output package name>: Specifies the app package or bundle.
:: ----------------------------

:step3_begin


"%MAKEAPPX%" pack /v /f "%OUTPUT_DIR%\mapping.txt" /p "%OUTPUT_DIR%\krita.msix" /o
if errorlevel 1 (
    echo ERROR running makeappx 1>&2
    exit /B 1
)

echo.
echo MSIX generated as "%OUTPUT_DIR%\krita.msix".

if "%SIGNTOOL_SIGN_FLAGS%" == "" goto skip_signing

echo Signing MSIX...
"%SIGNTOOL%" sign %SIGNTOOL_SIGN_FLAGS% /fd sha256 "%OUTPUT_DIR%\krita.msix"
if errorlevel 1 (
    echo ERROR running signtool 1>&2
    echo If you need to specify a PFX keyfile and its password, run:
    echo     set SIGNTOOL_SIGN_FLAGS=/f "absolute_path_to_keyfile.pfx" /p password
    exit /B 1
)

:skip_signing

echo === Step 3 done. ===

echo *** Script completed ***
