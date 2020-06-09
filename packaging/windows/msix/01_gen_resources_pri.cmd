@echo off

setlocal enableextensions enabledelayedexpansion

if "%WindowsSdkDir%" == "" if not "%ProgramFiles(x86)%" == "" set "WindowsSdkDir=%ProgramFiles(x86)%\Windows Kits\10"
if "%WindowsSdkDir%" == "" set "WindowsSdkDir=%ProgramFiles(x86)%\Windows Kits\10"
if exist "%WindowsSdkDir%\" (
    pushd "%WindowsSdkDir%"
    for /f "delims=" %%a in ('dir /a:d /b "bin\10.*"') do (
        if exist "bin\%%a\x64\makeappx.exe" (
            set "MAKEPRI=%WindowsSdkDir%\bin\%%a\x64\makepri.exe"
        )
    )
    if "%MAKEPRI%" == "" if exist "bin\x64\makepri.exe" (
        set "MAKEPRI=%WindowsSdkDir%\bin\x64\makepri.exe"
    )
    popd
)
if "%MAKEPRI%" == "" (
    echo ERROR: makepri not found 1>&2
    exit /b 1
)

mkdir out

"%MAKEPRI%" new /pr "%~dp0pkg" /mn "%~dp0manifest.xml" /cf "%~dp0priconfig.xml" /o /of "%~dp0out\resources.pri"
if errorlevel 1 (
    echo ERROR running makepri 1>&2
    exit /B 1
)

echo Done.
