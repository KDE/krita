:: This batch script is meant to prepare a Krita package folder to be zipped or
:: to be a base for the installer.
::
:: Just drop it next to the "i" install folder where the dependencies and Krita binaries are.
::
:: TODO: Ask if the user want to make an archive and with which tool
:: TODO: Maybe ask for a custom install folder name?
set ZIP="c:\Program Files\7-Zip"
set MINGW_GCC_BIN=c:\TDM-GCC-64\bin\
set BUILDROOT=%CD%

set BUILDDIR_INSTALL=%BUILDROOT%\i
set PATH=%MINGW_GCC_BIN%;%PATH%

@echo off

if not exist %BUILDDIR_INSTALL% (
echo Cannot find the install folder!
pause
exit /B
)

set /P pkg_root=Insert krita package name: 

if [%pkg_root%] == [] (
echo You entered an empty name!
pause
exit /B
)

:: Initial folder setup
mkdir %pkg_root%
mkdir %pkg_root%\bin
mkdir %pkg_root%\lib
mkdir %pkg_root%\share

:: Bin folder
copy %MINGW_GCC_BIN%\lib*.dll %pkg_root%\bin
copy %BUILDDIR_INSTALL%\bin\krita.exe %pkg_root%\bin
copy %BUILDDIR_INSTALL%\bin\*.dll %pkg_root%\bin
copy %BUILDDIR_INSTALL%\lib\*.dll %pkg_root%\bin
xcopy /S /Y /I %BUILDDIR_INSTALL%\plugins\imageformats %pkg_root%\bin\imageformats
xcopy /S /Y /I %BUILDDIR_INSTALL%\plugins\kf5 %pkg_root%\bin\kf5
xcopy /S /Y /I %BUILDDIR_INSTALL%\plugins\platforms\qwindows.dll %pkg_root%\bin\platforms\
xcopy /S /Y /I %BUILDDIR_INSTALL%\plugins\printsupport %pkg_root%\bin\printsupport
xcopy /Y %BUILDDIR_INSTALL%\plugins\iconengines\*.dll %pkg_root%\bin\iconengines\

:: Translations
mkdir %pkg_root%\bin\translations
copy %BUILDDIR_INSTALL%\translations\qt_ca.qm %pkg_root%\bin\translations\qt_ca.qm
copy %BUILDDIR_INSTALL%\translations\qt_cs.qm %pkg_root%\bin\translations\qt_cs.qm
copy %BUILDDIR_INSTALL%\translations\qt_de.qm %pkg_root%\bin\translations\qt_de.qm
copy %BUILDDIR_INSTALL%\translations\qt_en.qm %pkg_root%\bin\translations\qt_en.qm
copy %BUILDDIR_INSTALL%\translations\qt_fi.qm %pkg_root%\bin\translations\qt_fi.qm
copy %BUILDDIR_INSTALL%\translations\qt_he.qm %pkg_root%\bin\translations\qt_hu.qm
copy %BUILDDIR_INSTALL%\translations\qt_it.qm %pkg_root%\bin\translations\qt_it.qm
copy %BUILDDIR_INSTALL%\translations\qt_ja.qm %pkg_root%\bin\translations\qt_ja.qm
copy %BUILDDIR_INSTALL%\translations\qt_ko.qm %pkg_root%\bin\translations\qt_ko.qm
copy %BUILDDIR_INSTALL%\translations\qt_lv.qm %pkg_root%\bin\translations\qt_lv.qm
copy %BUILDDIR_INSTALL%\translations\qt_ru.qm %pkg_root%\bin\translations\qt_ru.qm
copy %BUILDDIR_INSTALL%\translations\qt_sk.qm %pkg_root%\bin\translations\qt_sk.qm
copy %BUILDDIR_INSTALL%\translations\qt_uk.qm %pkg_root%\bin\translations\qt_uk.qm
copy %BUILDDIR_INSTALL%\translations\qt_fr.qm %pkg_root%\bin\translations\qt_fr.qm

:: Lib
xcopy /Y %BUILDDIR_INSTALL%\lib\kritaplugins\*.dll %pkg_root%\lib\kritaplugins\
strip %pkg_root%\lib\kritaplugins\*.dll

:: Share
xcopy /Y /S /I %BUILDDIR_INSTALL%\share\appdata %pkg_root%\share\appdata
xcopy /Y /S /I %BUILDDIR_INSTALL%\share\applications %pkg_root%\share\applications
xcopy /Y /S /I %BUILDDIR_INSTALL%\share\color %pkg_root%\share\color
xcopy /Y /S /I %BUILDDIR_INSTALL%\share\color-schemes %pkg_root%\share\color-schemes
xcopy /Y /S /I %BUILDDIR_INSTALL%\share\doc %pkg_root%\share\doc
xcopy /Y /S /I %BUILDDIR_INSTALL%\share\icons %pkg_root%\share\icons
xcopy /Y /S /I %BUILDDIR_INSTALL%\share\kf5 %pkg_root%\share\kf5
xcopy /Y /S /I %BUILDDIR_INSTALL%\share\krita %pkg_root%\share\krita
xcopy /Y /S /I %BUILDDIR_INSTALL%\share\kritaplugins %pkg_root%\share\kritaplugins
xcopy /Y /S /I %BUILDDIR_INSTALL%\share\kservices5 %pkg_root%\share\kservices5
xcopy /Y /S /I %BUILDDIR_INSTALL%\share\locale %pkg_root%\bin\locale
xcopy /Y /S /I %BUILDDIR_INSTALL%\share\man %pkg_root%\share\man
xcopy /Y /S /I %BUILDDIR_INSTALL%\share\mime %pkg_root%\share\mime
xcopy /Y /S /I %BUILDDIR_INSTALL%\share\ocio %pkg_root%\share\ocio

::Link
copy %BUILDROOT%\krita\packaging\windows\krita.lnk %pkg_root%

%BUILDDIR_INSTALL%\bin\windeployqt.exe %pkg_root%\bin\krita.exe

:: Debug build

%ZIP%\7z.exe a -tzip %pkg_root%-dbg.zip %pkg_root%

:: Bin folder
strip %pkg_root%\bin\krita.exe
strip %pkg_root%\bin\*.dll
strip %pkg_root%\bin\imageformats\*.dll
strip %pkg_root%\bin\kf5\*.dll
strip %pkg_root%\bin\kf5\org.kde.kwindowsystem.platforms\*.dll
strip %pkg_root%\bin\platforms\*.dll
strip %pkg_root%\bin\iconengines\*.dll

%ZIP%\7z.exe a -tzip %pkg_root%.zip %pkg_root%
