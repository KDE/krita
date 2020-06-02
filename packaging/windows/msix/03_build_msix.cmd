@echo off

setlocal enableextensions enabledelayedexpansion
goto begin

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


:begin


if "%WindowsSdkDir%" == "" if not "%ProgramFiles(x86)%" == "" set "WindowsSdkDir=%ProgramFiles(x86)%\Windows Kits\10"
if "%WindowsSdkDir%" == "" set "WindowsSdkDir=%ProgramFiles(x86)%\Windows Kits\10"
if exist "%WindowsSdkDir%\" (
    pushd "%WindowsSdkDir%"
    for /f "delims=" %%a in ('dir /a:d /b "bin\10.*"') do (
        if exist "bin\%%a\x64\makeappx.exe" (
            set "MAKEAPPX=%WindowsSdkDir%\bin\%%a\x64\makeappx.exe"
        )
        if exist "bin\%%a\x64\signtool.exe" (
            set "SIGNTOOL=%WindowsSdkDir%\bin\%%a\x64\signtool.exe"
        )
    )
    if "%MAKEAPPX%" == "" if exist "bin\x64\makeappx.exe" (
        set "MAKEAPPX=%WindowsSdkDir%\bin\x64\makeappx.exe"
    )
    if "%SIGNTOOL%" == "" if exist "bin\x64\signtool.exe" (
        set "SIGNTOOL=%WindowsSdkDir%\bin\x64\signtool.exe"
    )
    popd
)
if "%MAKEAPPX%" == "" (
    echo ERROR: makeappx not found 1>&2
    exit /b 1
)
if "%SIGNTOOL%" == "" (
    echo ERROR: signtool not found 1>&2
    exit /b 1
)


"%MAKEAPPX%" pack /v /f "%~dp0out\mapping.txt" /p "%~dp0out\out.msix" /o
if errorlevel 1 (
    echo ERROR running makeappx 1>&2
    exit /B 1
)

echo.
echo MSIX generated. Signing...

"%SIGNTOOL%" sign %SIGNTOOL_SIGN_FLAGS% /fd sha256 "%~dp0out\out.msix"
if errorlevel 1 (
    echo ERROR running signtool 1>&2
    echo If you need to specify a PFX keyfile and its password, run:
    echo     set SIGNTOOL_SIGN_FLAGS=/f "absolute_path_to_keyfile.pfx" /p password
    exit /B 1
)

echo Done.
