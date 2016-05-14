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
::  C2W Installer 
::  Environment settings
:: 
::  This file checks for the presence of environment variables
::  and sets default values if they do not exist.
::
::  wix process
::  -----------
::  C2WINSTALL_WIX_BIN - location of the Wix binaries
::  C2WINSTALL_OUTPUT - destination for generated installers
::  C2WINSTALL_INPUT - source directory of packaged KDEROOT+CALLIGRA_INST
::  C2WINSTALL_MERGEMODS - directory where we should look for Merge Modules
::
::  package process
::  ---------------
::  CALLIGRA_INST - the Calligra installation directory
::  KDEROOT - the root of the KDE on Windows emerge based distribution
:: 
::  Build options (defined elsewhere)
::  ---------------------------------
:: C2WINSTALL_VC2010_DISTRIBUTE:
::     DLL - bundle DLLs
::     MSM - use Merge Modules
::     <other> - display error message  
::
:: ...USE_DLL has priority, if neither are defined, an error is displayed
::
::  -----------------------------------------------------------------------
::
set C2WINSTALL_GITREV=last
set C2WINSTALL_VERSION=2.6.8.0
set C2WINSTALL_VERSIONSTRING="2.6.8.0"
set CALLIGRA_INST=u:
set KDEROOT=u:
set C2WINSTALL_VC2010_DISTRIBUTE=MSM

IF "%C2WINSTALL_VERSION%"=="" (
    :: We must define this, if it hasn't already been
        echo WARNING: You should set C2WINSTALL_VERSION
        set C2WINSTALL_VERSION=0.0.0.0
)

IF "%C2WINSTALL_GITREV%"=="" (
        :: We must define this, if it hasn't already been
        echo WARNING: You should set C2WINSTALL_GITREV
        set C2WINSTALL_GITREV=last
)

IF "%C2WINSTALL_VC2010_DISTRIBUTE%"=="" (
        :: If no distribute set, then need to set to 
        :: something in order to display warning dialog.
        set C2WINSTALL_VC2010_DISTRIBUTE=ERROR
)

IF "%C2WINSTALL_WIX_BIN%"=="" (
        set C2WINSTALL_WIX_BIN=%~dp0..\wix36
)

IF "%C2WINSTALL_MERGEMODULES%"=="" (
        set C2WINSTALL_MERGEMODULES=%~dp0..\deps
)

IF "%C2WINSTALL_VC2010_DLLS%"=="" (
        set C2WINSTALL_VC2010_DLLS=%~dp0..\deps\vcredist\DLLs\Microsoft.VC100.CRT
)

IF "%C2WINSTALL_TEMP%"=="" (
        set C2WINSTALL_TEMP=%~dp0..\c2winstaller-temp
)

IF "%C2WINSTALL_INPUT%"=="" (
        set C2WINSTALL_INPUT=%~dp0..\c2winstaller-input
)

IF "%C2WINSTALL_OUTPUT%"=="" (
        set C2WINSTALL_OUTPUT=%~dp0..\c2winstaller-output
)

IF "%CALLIGRA_INST%"=="" (
        set CALLIGRA_INST=%~dp0..\kde4\inst
)

IF "%KDEROOT%"=="" (
        set KDEROOT=%~dp0..\kderoot
)

SET PATH=%C2WINSTALL_WIX_BIN%;%PATH%

