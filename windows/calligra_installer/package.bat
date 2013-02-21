:: Copyright (c) 2011-2012 KO GmbH.  All rights reserved.
:: Copyright (c) 2011-2012 Stuart Dickson <stuartmd@kogmbh.com>
::
:: The use and distribution terms for this software are covered by the
:: Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
:: which can be found in the file CPL.TXT at the root of this distribution.
:: By using this software in any fashion, you are agreeing to be bound by
:: the terms of this license.
::  
:: You must not remove this notice, or any other, from this software.
:: ------------------------------------------------------------------------
::
::  package_calligra.bat
::
::  Copies relevant files from the calligra inst and kderoot folders 
::
@echo off

::  Safety check:
::  Make sure key variables are defined

IF "%C2WINSTALL_INPUT%"=="" (
	echo !!! C2WINSTALL VARIABLES NOT PROPERLY DEFINED !!!
	echo Operation cancelled.
	goto :eof
	pause
)
IF "%C2WINSTALL_TEMP%"=="" (
	echo !!! C2WINSTALL VARIABLES NOT PROPERLY DEFINED !!!
	echo Operation cancelled.
	goto :eof
	pause
)
IF "%C2WINSTALL_OUTPUT%"=="" (
	echo !!! C2WINSTALL VARIABLES NOT PROPERLY DEFINED !!!
	echo Operation cancelled.

	goto :eof
	pause
)

::  Test for argument
IF "%1"==""  (
	@echo off
) ELSE (
	mkdir %C2WINSTALL_INPUT%\output
	del %C2WINSTALL_INPUT%\output\*.* /S /Q
	SET CALLIGRA_INST=%C2WINSTALL_INPUT%\%1\inst
	SET KDEROOT=%C2WINSTALL_INPUT%\%1\kderoot
	SET C2WINSTALL_INPUT=%C2WINSTALL_INPUT%\output
)

::  Create redistribution directories
mkdir %c%
mkdir %C2WINSTALL_INPUT%
mkdir %C2WINSTALL_OUTPUT%
del %C2WINSTALL_TEMP%\*.* /Q /S
del %C2WINSTALL_INPUT%\*.* /Q /S

mkdir %C2WINSTALL_INPUT%\bin
mkdir %C2WINSTALL_INPUT%\etc
mkdir %C2WINSTALL_INPUT%\lib
mkdir %C2WINSTALL_INPUT%\plugins
mkdir %C2WINSTALL_INPUT%\share

echo Copying Calligra binaries...
xcopy "%CALLIGRA_INST%\bin\*.*" "%C2WINSTALL_TEMP%" /Q /Y /EXCLUDE:%~dp0res\package\todelete.txt
echo .
move "%C2WINSTALL_TEMP%\*.exe" "%C2WINSTALL_INPUT%\bin"
move "%C2WINSTALL_TEMP%\*.dll" "%C2WINSTALL_INPUT%\bin"
echo ..
move "%C2WINSTALL_TEMP%\*.*" "%C2WINSTALL_INPUT%\lib"
echo ...
xcopy "%CALLIGRA_INST%\lib\kde4" "%C2WINSTALL_INPUT%\lib\kde4\" /Q /Y /EXCLUDE:%~dp0res\package\todelete.txt
echo ....
xcopy "%CALLIGRA_INST%\share" "%C2WINSTALL_INPUT%\share" /I /E /Q /Y /EXCLUDE:%~dp0res\package\todelete.txt
echo .....

echo Copying KDE-Windows binaries
xcopy "%KDEROOT%\bin" "%C2WINSTALL_TEMP%\bin" /I /E /Q /Y /EXCLUDE:%~dp0res\package\todelete.txt
xcopy "%KDEROOT%\etc" "%C2WINSTALL_TEMP%\etc" /I /E /Q /Y /EXCLUDE:%~dp0res\package\todelete.txt
xcopy "%KDEROOT%\lib" "%C2WINSTALL_TEMP%\lib" /I /E /Q /Y /EXCLUDE:%~dp0res\package\todelete.txt
xcopy "%KDEROOT%\plugins" "%C2WINSTALL_TEMP%\plugins" /I /E /Q /Y /EXCLUDE:%~dp0res\package\todelete.txt
xcopy "%KDEROOT%\share" "%C2WINSTALL_TEMP%\share" /I /E /Q /Y /EXCLUDE:%~dp0res\package\todelete.txt

del "%C2WINSTALL_TEMP%\lib\*.prl" /S /Q
del "%C2WINSTALL_TEMP%\lib\*.cmd" /S /Q
del "%C2WINSTALL_TEMP%\lib\*.bat" /S /Q
del "%C2WINSTALL_TEMP%\lib\*.def" /S /Q
del "%C2WINSTALL_TEMP%\lib\libpng" /S /Q
del "%C2WINSTALL_TEMP%\lib\libstreamanalyzer" /S /Q
del "%C2WINSTALL_TEMP%\lib\libstreams" /S /Q


move "%C2WINSTALL_TEMP%\lib\dbus*.*" "%C2WINSTALL_TEMP%\bin\"
move "%C2WINSTALL_TEMP%\lib\*.dll" "%C2WINSTALL_TEMP%\bin\"

xcopy "%C2WINSTALL_TEMP%" "%C2WINSTALL_INPUT%" /Y /S /EXCLUDE:%~dp0res\package\todelete.txt

move "%C2WINSTALL_INPUT%\lib\libwmf.dll""%C2WINSTALL_INPUT%\bin\libwmf.dll
move "%C2WINSTALL_INPUT%\lib\msooxml.dll" %C2WINSTALL_INPUT%\bin\msooxml.dll
move "%C2WINSTALL_INPUT%\lib\RtfReader.dll" %C2WINSTALL_INPUT%\bin\RtfReader.dll

::  Temporary Measure - rename kspread.png to sheets.png in hicolor
::  TODO Find out why KDE looks for sheets instead of kspread
ren "%C2WINSTALL_INPUT%\share\icons\hicolor\16x16\apps\kspread.png sheets.png"
ren "%C2WINSTALL_INPUT%\share\icons\hicolor\22x22\apps\kspread.png sheets.png"
ren "%C2WINSTALL_INPUT%\share\icons\hicolor\32x32\apps\kspread.png sheets.png"
ren "%C2WINSTALL_INPUT%\share\icons\hicolor\48x48\apps\kspread.png sheets.png"

:: ICONV.DLL improve
:: TODO: consider restructuring the install directory
:: move %C2WINSTALL_INPUT%\lib\iconv.dll %C2WINSTALL_INPUT%\bin\iconv.dll

::  Delete unused resources
::  TODO in future we will package .pdb 
del "%C2WINSTALL_TEMP%\*.*" /S /Q

echo Complete.
