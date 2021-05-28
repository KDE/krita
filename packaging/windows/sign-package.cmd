@echo off
setlocal enableextensions enabledelayedexpansion

set pkg_root=%~f1

if not "%SIGNTOOL%" == "" goto skip_find_signtool

:: Find Windows SDK for signtool.exe
if "%WindowsSdkDir%" == "" if not "%ProgramFiles(x86)%" == "" set "WindowsSdkDir=%ProgramFiles(x86)%\Windows Kits\10"
if "%WindowsSdkDir%" == "" set "WindowsSdkDir=%ProgramFiles(x86)%\Windows Kits\10"
if exist "%WindowsSdkDir%\" (
    pushd "%WindowsSdkDir%"
    for /f "delims=" %%a in ('dir /a:d /b "bin\10.*"') do (
        if exist "bin\%%a\x64\signtool.exe" (
            set "SIGNTOOL=%WindowsSdkDir%\bin\%%a\x64\signtool.exe"
        )
    )
    if "%SIGNTOOL%" == "" if exist "bin\x64\signtool.exe" (
        set "SIGNTOOL=%WindowsSdkDir%\bin\x64\signtool.exe"
    )
    popd
)
if "%SIGNTOOL%" == "" (
    echo ERROR: signtool not found 1>&2
    exit /b 1
)

:skip_find_signtool

if "!SIGNTOOL_SIGN_FLAGS!" == "" (
    echo ERROR: Please set environment variable SIGNTOOL_SIGN_FLAGS 1>&2
    exit /b 1
    :: This is what I used for testing:
    :: set "SIGNTOOL_SIGN_FLAGS=/f "C:\Users\Alvin\MySPC.pfx" /t http://timestamp.verisign.com/scripts/timstamp.dll"
)

echo Signing binaries in "%pkg_root%"
if not exist "%pkg_root%\" (
    echo ERROR: No packaging dir %pkg_root% 1>&2
    exit /b 1
)
for /r "%pkg_root%\" %%F in (*.exe *.com *.dll *.pyd) do (
    :: Check for existing signature
    "%SIGNTOOL%" verify /q /pa "%%F" > NUL
    if errorlevel 1 (
        echo Signing %%F
        "%SIGNTOOL%" sign %SIGNTOOL_SIGN_FLAGS% "%%F"
        if errorlevel 1 (
            echo ERROR: Got exit code !errorlevel! from signtool! 1>&2
            exit /b 1
        )
    ) else (
        echo Not signing %%F - file already signed
    )
)
endlocal
