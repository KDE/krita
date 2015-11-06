# Copyright (c) 2011-2012 KO GmbH.  All rights reserved.
# Copyright (c) 2011-2012 Stuart Dickson <stuartmd@kogmbh.com>
# Copyright (c) 2015 Michael Abrahams <miabraha@gmail.com>


# The use and distribution terms for this software are covered by the
# Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
# which can be found in the file CPL.TXT at the root of this distribution.
# By using this software in any fashion, you are agreeing to be bound by
# the terms of this license.

# You must not remove this notice, or any other, from this software.
# ------------------------------------------------------------------------

# build_msi.bat


# TODO: disable UpdateMimeDB and KBuildSycoca in krita.wxs
# TODO: look into xdg and etc directories to see what's needed

# Clean up from last time.
del "$env:KRITA_OUTPUT\*.wixobj"

$KRITA_INSTALLER_FILENAME="$env:KRITA_OUTPUT\krita_$env:KRITA_VERSION"

# Collects contents of $KRITA_INPUT. "KRITADIR" is a tag for the installation directory used inside the wxs file
echo "Locating files..."
."$env:KRITA_WIX_BIN\heat.exe" dir "$env:KRITA_INPUT" -cg CalligraBaseFiles -gg -scom -sreg -sfrag -srd -dr KRITADIR -var env.KRITA_INPUT -out "HeatFragment.wxs"


# Configures installation instructions & metadata. x64 only, for now. This uses
# the XML configuration files contained in in res\, and bakes in paths using
# res\package\env.bat

# This uses x64 architecture.
# To make it work for 32 bits, I believe you want to change these things:
# -Remove "-arch x64" and change "ProgramFiles64Folder"

$KRITA_ARCH = "x64"

echo "Building metadata..."
."$env:KRITA_WIX_BIN\candle.exe" -arch x64 `
  HeatFragment.wxs krita.wxs `
  res\wix_snippets\BundleVC2015x64.wxs `
  res\UIExtension\CustomUI_Mondo.wxs `
  res\UIExtension\CustomProgressDlg.wxs `
  res\UIExtension\CustomWelcomeDlg.wxs `
  res\UIExtension\CustomMaintenanceWelcomeDlg.wxs `
  res\UIExtension\CustomResumeDlg.wxs -out $env:KRITA_OUTPUT\\


echo "Creating executable..."
."$env:KRITA_WIX_BIN\light.exe" -nologo  "$env:KRITA_OUTPUT\*.wixobj" `
  -out "$KRITA_INSTALLER_FILENAME.msi" -ext WixUIExtension


Get-FileHash -Algorithm SHA1 "$KRITA_INSTALLER_FILENAME.msi" > "$KRITA_INSTALLER_FILENAME.sha1"


echo "Complete."
