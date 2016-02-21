:: This batch script is meant to prepare a Krita package folder to be zipped or
:: to be a base for the installer.
::
:: Just drop it next to the "i" install folder where the dependencies and Krita binaries are.
::
:: TODO: Ask if the user want to make an archive and with which tool
:: TODO: Maybe ask for a custom install folder name?

@echo off

if not exist i\ (
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
copy i\bin\krita.exe %pkg_root%\bin
copy i\bin\*.dll %pkg_root%\bin
copy i\lib\*.dll %pkg_root%\bin
xcopy /S /Y /I i\lib\plugins\imageformats %pkg_root%\bin\imageformats
xcopy /S /Y /I i\lib\plugins\kf5 %pkg_root%\bin\kf5
xcopy /S /Y /I i\plugins\platforms\qwindows.dll %pkg_root%\bin\platforms\
xcopy /Y i\plugins\iconengines\*.dll %pkg_root%\bin\iconengines\

:: Translations
mkdir %pkg_root%\bin\translations
copy i\translations\qt_ca.qm %pkg_root%\bin\translations\qt_ca.qm
copy i\translations\qt_cs.qm %pkg_root%\bin\translations\qt_cs.qm
copy i\translations\qt_de.qm %pkg_root%\bin\translations\qt_de.qm
copy i\translations\qt_en.qm %pkg_root%\bin\translations\qt_en.qm
copy i\translations\qt_fi.qm %pkg_root%\bin\translations\qt_fi.qm
copy i\translations\qt_he.qm %pkg_root%\bin\translations\qt_hu.qm
copy i\translations\qt_it.qm %pkg_root%\bin\translations\qt_it.qm
copy i\translations\qt_ja.qm %pkg_root%\bin\translations\qt_ja.qm
copy i\translations\qt_ko.qm %pkg_root%\bin\translations\qt_ko.qm
copy i\translations\qt_lv.qm %pkg_root%\bin\translations\qt_lv.qm
copy i\translations\qt_ru.qm %pkg_root%\bin\translations\qt_ru.qm
copy i\translations\qt_sk.qm %pkg_root%\bin\translations\qt_sk.qm
copy i\translations\qt_uk.qm %pkg_root%\bin\translations\qt_uk.qm
copy i\translations\qt_fr.qm %pkg_root%\bin\translations\qt_fr.qm

:: Lib
xcopy /Y i\lib\kritaplugins\*.dll %pkg_root%\lib\kritaplugins\

:: Share
xcopy /Y /S /I i\share\appdata %pkg_root%\share\appdata
xcopy /Y /S /I i\share\applications %pkg_root%\share\applications
xcopy /Y /S /I i\share\color %pkg_root%\share\color
xcopy /Y /S /I i\share\color-schemes %pkg_root%\share\color-schemes
xcopy /Y /S /I i\share\doc %pkg_root%\share\doc
xcopy /Y /S /I i\share\icons %pkg_root%\share\icons
xcopy /Y /S /I i\share\kf5 %pkg_root%\share\kf5
xcopy /Y /S /I i\share\krita %pkg_root%\share\krita
xcopy /Y /S /I i\share\kritaplugins %pkg_root%\share\kritaplugins
xcopy /Y /S /I i\share\kservices5 %pkg_root%\share\kservices5
xcopy /Y /S /I i\share\locale %pkg_root%\share\locale
xcopy /Y /S /I i\share\man %pkg_root%\share\man
xcopy /Y /S /I i\share\mime %pkg_root%\share\mime
xcopy /Y /S /I i\share\ocio %pkg_root%\share\ocio

::Link
mklink %pkg_root%\krita.exe bin\krita.exe
