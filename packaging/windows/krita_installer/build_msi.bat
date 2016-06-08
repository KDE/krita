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

set C2WINSTALL_TEMP_INSTALLER=%C2WINSTALL_OUTPUT%\krita_%C2WINSTALL_VERSION%

::  call %~dp0run_heat.bat
m:\package\wix36\candle.exe %~dp0HeatFragment.wxs %~dp0krita.wxs %~dp0res\wix_snippets\BundleVC2010x86.wxs %~dp0res\UIExtension\CustomUI_Mondo.wxs %~dp0res\UIExtension\CustomProgressDlg.wxs %~dp0res\UIExtension\CustomWelcomeDlg.wxs %~dp0res\UIExtension\CustomMaintenanceWelcomeDlg.wxs %~dp0res\UIExtension\CustomResumeDlg.wxs -out %C2WINSTALL_OUTPUT%\\

echo.
echo Running light...
m:\package\wix36\light.exe -nologo "%C2WINSTALL_OUTPUT%\*.wixobj" -out "%C2WINSTALL_TEMP_INSTALLER%.msi"  -ext WixUIExtension  
shasum "%C2WINSTALL_TEMP_INSTALLER%.msi" > "%C2WINSTALL_TEMP_INSTALLER%.sh1"
::  Delete intermediate file 
::  It is possible for candle to fail without an error and hence
::  we should delete the wixobj file in case light.exe picks up
::  an old version 
::
echo Complete.
echo.
popd
