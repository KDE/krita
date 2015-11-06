# Copyright (c) 2011-2012 KO GmbH.  All rights reserved.
# Copyright (c) 2011-2012 Stuart Dickson <stuartmd@kogmbh.com>
# Copyright (c) 2015 Michael Abrahams <miabraha@gmail.com>
#
# The use and distribution terms for this software are covered by the
# Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
# which can be found in the file CPL.TXT at the root of this distribution.
# By using this software in any fashion, you are agreeing to be bound by
# the terms of this license.
#
# You must not remove this notice, or any other, from this software.
# ------------------------------------------------------------------------
#
#  Krita Windows Installer
#  Environment settings
#
#  This file checks for the presence of environment variables
#  and sets default values if they do not exist.
#
#  See README.txt for more details.
#
#  wix process
#  -----------
#  KRITA_WIX_BIN - location of the Wix binaries
#  KRITA_OUTPUT - destination for generated installers
#  KRITA_INPUT - source directory of packaged KDEROOT+CALLIGRA_INST
#  KRITA_MERGEMODS - directory where we should look for Merge Modules
#
#  package process
#  ---------------
#  CALLIGRA_INST - the Krita installation directory (TODO: rename)
#  KDEROOT - the root of the KDE on Windows emerge based distribution
#
#  Build options (defined elsewhere)
#  ---------------------------------
#
# ...USE_DLL has priority, if neither are defined, an error is displayed
#
#  -----------------------------------------------------------------------
#

# WIX reads environment variables, but PowerShell variables are local by
# default, so we have to prefix all variables with $env:
$env:KRITA_VERSION="3.0.0.1"
$env:KRITA_VERSIONSTRING="3.0.0.1"  # This could be set to a cute name if you wanted.
$env:KRITA_GITREV="last"


## Base installation directories
$env:KDEROOT="r:"
$env:CALLIGRA_INST="r:"
$PACKAGER_ROOT="r:\inst"  # Working directory for package.ps1 (script-local)
$env:KRITA_WIX_BIN="r:\inst\wix310"  # Location of WIX binaries



# Method of distributing MSVC 2015
#     DLL - bundle DLLs
#     MSM - use Merge Modules
#     <other> - error
$env:KRITA_VC2015_DISTRIBUTE="MSM"

# See MSDN: "Redistributing Components By Using Merge Modules"
# https://msdn.microsoft.com/en-us/library/ms235290.aspx
$env:KRITA_MERGEMODULES="C:\Program Files (x86)\Common Files\Merge Modules"


function warn-unset($v) {
    # Use this function only when there is no acceptable default.
    $val = (Get-Variable $v).value
    iex "echo ""Warning: variable $v was not set.  Please check your configuration."
    Exit
}

if ($env:CALLIGRA_INST -eq $null) {
    warn-unset "CALLIGRA_INST"
}

if ($env:KDEROOT -eq $null) {
    warn-unset "$env:KDEROOT"
}

if ($env:KRITA_VERSION -eq $null) {
    warn-unset "KRITA_VERSION"
    Exit
}

if ($env:KRITA_GITREV -eq $null) {
    warn-unset "KRITA_GITREV"
}


$env:KRITA_VC2015_DLLS="$PWD\..\deps\vcredist\DLLs\Microsoft.VC140.CRT"
$env:KRITA_TEMP="$PACKAGER_ROOT\installer-temp"
$env:KRITA_INPUT="$PACKAGER_ROOT\installer-input"
$env:KRITA_OUTPUT="$PACKAGER_ROOT\installer-output"



# set $env:Path "$($env:Path);$KRITA_WIX_BIN"
