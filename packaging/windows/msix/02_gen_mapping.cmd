@echo off

:: Please edit this to point to the extracted installer files.
set "KRITA_DIR=c:\dev\krita-x64-4.4.2-setup"



set "ASSETS_DIR=%~dp0pkg\Assets"
set "MAPPING_OUT=%~dp0out\mapping.txt"

setlocal enabledelayedexpansion
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


:: ----------------------------
:begin


rem Sanity checks:
if not exist "%KRITA_DIR%\bin\krita.exe" (
    echo ERROR: KRITA_DIR is set to "%KRITA_DIR%" but "%KRITA_DIR%\bin\krita.exe" does not exist! 1>&2
    exit /B 1
)
if not exist "%KRITA_DIR%\shellex\kritashellex64.dll" (
    echo ERROR: "%KRITA_DIR%\shellex\kritashellex64.dll" does not exist! 1>&2
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


call :get_temp_file OUT_TEMP
echo Writing list to temporary file "%OUT_TEMP%"

echo [Files] > %OUT_TEMP%
echo "%~dp0manifest.xml" "AppxManifest.xml" >> %OUT_TEMP%
echo "%~dp0out\resources.pri" "Resources.pri" >> %OUT_TEMP%

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
echo Done.
