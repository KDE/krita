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
#  C2W Installer
#  Environment settings
#
#  This file checks for the presence of environment variables
#  and sets default values if they do not exist.
#
#  wix process
#  -----------
#  C2WINSTALL_WIX_BIN - location of the Wix binaries
#  C2WINSTALL_OUTPUT - destination for generated installers
#  C2WINSTALL_INPUT - source directory of packaged KDEROOT+CALLIGRA_INST
#  C2WINSTALL_MERGEMODS - directory where we should look for Merge Modules
#
#  package process
#  ---------------
#  CALLIGRA_INST - the Calligra installation directory
#  KDEROOT - the root of the KDE on Windows emerge based distribution
#
#  Build options (defined elsewhere)
#  ---------------------------------
# C2WINSTALL_VC2010_DISTRIBUTE:
#     DLL - bundle DLLs
#     MSM - use Merge Modules
#     <other> - display error message
#
# ...USE_DLL has priority, if neither are defined, an error is displayed
#
#  -----------------------------------------------------------------------
#
$C2WINSTALL_GITREV="last"
$C2WINSTALL_VERSION="2.6.8.0"
$C2WINSTALL_VERSIONSTRING="2.6.8.0"


## Uncomment these to set manually
$CALLIGRA_INST="r:"
$KDEROOT="r:"
$C2WINSTALL_ROOT="$KDEROOT\inst"


$C2WINSTALL_VC2010_DISTRIBUTE="MSM"


function warn-unset($v) {
    # Use this function only when there is no acceptable default
    $val = (Get-Variable $v).value
    iex "echo ""Warning: $v not found. Set to default '$val'"""
}

if ($CALLIGRA_INST -eq $null) {
    $CALLIGRA_INST="$PWD..\kde4\inst"
    warn-unset "CALLIGRA_INST"
}

if ($C2WINSTALL_ROOT -eq $null) {
    $C2WINSTALL_ROOT="$PWD\.."
    warn-unset "C2WINSTALL_ROOT"
}

if ($env:KDEROOT -eq $null) {
    $env:KDEROOT="$PWD..\kderoot"
    warn-unset "$env:KDEROOT"
}

if ($C2WINSTALL_VERSION -eq $null) {
    $C2WINSTALL_VERSION="0.0.0.0"
    warn-unset "C2WINSTALL_VERSION"
}

if ($C2WINSTALL_GITREV -eq $null) {
    $C2WINSTALL_GITREV="last"
    warn-unset "C2WINSTALL_GITREV"
}

if ($C2WINSTALL_VC2010_DISTRIBUTE -eq $null) {
    $C2WINSTALL_VC2010_DISTRIBUTE="ERROR"
    warn-unset "C2WINSTALL_VC2010_DISTRIBUTE"
}

if ($C2WINSTALL_WIX_BIN -eq $null) {
    $C2WINSTALL_WIX_BIN="$C2WINSTALL_ROOT\wix36"
}

if ($C2WINSTALL_MERGEMODULES -eq $null) {
    $C2WINSTALL_MERGEMODULES="$C2WINSTALL_ROOT\deps"
}

if ($C2WINSTALL_VC2010_DLLS -eq $null) {
    $C2WINSTALL_VC2010_DLLS="$PWD\..\deps\vcredist\DLLs\Microsoft.VC100.CRT"
}

if ($C2WINSTALL_TEMP -eq $null) {
    $C2WINSTALL_TEMP="$C2WINSTALL_ROOT\c2winstaller-temp"
}

if ($C2WINSTALL_INPUT -eq $null) {
    $C2WINSTALL_INPUT="$C2WINSTALL_ROOT\c2winstaller-input"
}

if ($C2WINSTALL_OUTPUT -eq $null) {
    $C2WINSTALL_OUTPUT="$C2WINSTALL_ROOT\c2winstaller-output"
}

set $env:Path "$($env:Path);$C2WINSTALL_WIX_BIN"
