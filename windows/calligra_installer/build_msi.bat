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
:: build_msi.bat
:: 
::

@echo off
pushd %~dp0
set PATH=%WIX_BIN%;%PATH%
del "%C2WINSTALL_OUTPUT%\*.wixobj"

:: get version info from the header file
python getcalligraversion.py >tempverh.bat
call tempverh.bat
echo Calligra Version String: %C2WINSTALL_VERSIONSTRING%
echo According to header, version is: %C2WINSTALL_HEADERVERSIONSEP%
echo Filename will be of form calligra_%C2WINSTALL_HEADERVERSION%_x.x.x.x
set C2WINSTALL_INSTALLER_SUFFIX=%C2WINSTALL_HEADERVERSION%_

set C2WINSTALL_TEMP_INSTALLER=%C2WINSTALL_OUTPUT%\calligra%C2WINSTALL_INSTALLER_SUFFIX%%C2WINSTALL_VERSION%

:: check for existence of built executables
:: this way we do not need to rely on the user setting correct parameters
cscript //Nologo identifybuiltcomponents.vbs >tempbuiltapps.bat
call tempbuiltapps.bat

::  call %~dp0run_heat.bat
candle.exe %~dp0HeatFragment.wxs %~dp0calligra.wxs %~dp0res\wix_snippets\BundleVC2010x86.wxs %~dp0res\UIExtension\CustomUI_Mondo.wxs %~dp0res\UIExtension\CustomProgressDlg.wxs %~dp0res\UIExtension\CustomWelcomeDlg.wxs %~dp0res\UIExtension\CustomMaintenanceWelcomeDlg.wxs %~dp0res\UIExtension\CustomResumeDlg.wxs -out %C2WINSTALL_OUTPUT%\\

echo.
echo Running light...
light.exe -loc en-us.wxl -cultures:en-us -nologo "%C2WINSTALL_OUTPUT%\*.wixobj" -out "%C2WINSTALL_TEMP_INSTALLER%.msi"  -ext WixUIExtension  

:: echo Running shasum...
%KDEROOT%\dev-utils\bin\sha1sum %C2WINSTALL_TEMP_INSTALLER%.msi > %C2WINSTALL_TEMP_INSTALLER%.sh1
::  Delete intermediate file 
::  It is possible for candle to fail without an error and hence
::  we should delete the wixobj file in case light.exe picks up
::  an old version 
::
echo Complete.
echo.
popd
