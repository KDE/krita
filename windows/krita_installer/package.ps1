#  package_calligra.bat
#
#  Copies relevant files from the calligra inst and kderoot folders
#

#  TODO: check that /etc/dbus-1/ is properly installed
#  TODO: check that icons are properly installed
#  TODO: check that /share/apps/ is properly installed
#  TODO: check share\\kde4\\services is properly installed

#  Safety check:
#  Make sure key variables are defined

if ($C2WINSTALL_INPUT -eq $null) {
  echo "!!! C2WINSTALL VARIABLES NOT PROPERLY DEFINED !!!"
  echo "Operation cancelled."
  Exit
}

if ($C2WINSTALL_INPUT -eq $null) {
  echo "!!! C2WINSTALL VARIABLES NOT PROPERLY DEFINED !!!"
  echo "Operation cancelled."
  Exit
}


# rm -r "$C2WINSTALL_INPUT"
# Create redistribution directories ( )

mkdir "$C2WINSTALL_INPUT" -ea SilentlyContinue
mkdir "$C2WINSTALL_OUTPUT" -ea SilentlyContinue
mkdir "$C2WINSTALL_INPUT\bin" -ea SilentlyContinue
mkdir "$C2WINSTALL_INPUT\etc" -ea SilentlyContinue
mkdir "$C2WINSTALL_INPUT\lib" -ea SilentlyContinue
mkdir "$C2WINSTALL_INPUT\lib\kde4" -ea SilentlyContinue
mkdir "$C2WINSTALL_INPUT\plugins" -ea SilentlyContinue
mkdir "$C2WINSTALL_INPUT\share" -ea SilentlyContinue
mkdir "$C2WINSTALL_INPUT\lib\kde4\styles" -ea SilentlyContinue
mkdir "$C2WINSTALL_INPUT\lib\kde4\imageformats" -ea SilentlyContinue
mkdir "$C2WINSTALL_INPUT\lib\kde4\styles\imageformats" -ea SilentlyContinue
mkdir "$C2WINSTALL_INPUT\share\applications\kde4" -ea SilentlyContinue
mkdir "$C2WINSTALL_INPUT\share\applications\kde4\apps" -ea SilentlyContinue
mkdir "$C2WINSTALL_INPUT\share\config" -ea SilentlyContinue
mkdir "$C2WINSTALL_INPUT\etc\dbus-1\" -ea SilentlyContinue
mkdir "$C2WINSTALL_INPUT\share\dbus-1\" -ea SilentlyContinue
mkdir "$C2WINSTALL_INPUT\share\dbus-1\interfaces" -ea SilentlyContinue
mkdir "$C2WINSTALL_INPUT\share\dbus-1\services" -ea SilentlyContinue
mkdir "$C2WINSTALL_INPUT\share\dbus-1\system-services"-ea SilentlyContinue
mkdir "$C2WINSTALL_INPUT\share\kde4\services"-ea SilentlyContinue
mkdir "$C2WINSTALL_INPUT\share\kde4\servicetypes"-ea SilentlyContinue


$installinfo = ConvertFrom-Json ((Get-Content "krita-files.json") -join "`n")


function copy-newer($src, $dst) {
    # Copy items only if they are newer, return error string on failure
    Try {
        $src_item = Get-Item $src -ErrorAction SilentlyContinue
        if (-not $?) {
            return "Could not locate $src"
        }
        $dst_item = Get-Item $dst -ErrorAction SilentlyContinue
        if (-not $?) {
            $src_update = [datetime](Get-ItemProperty -Path $src -Name LastWriteTime)[0].LastWriteTime
            $dst_update = [datetime](Get-ItemProperty -Path $dst -Name LastWriteTime)[0].LastWriteTime
            if ($src_update -le $dst_update) {
                return ""
            }
        }
        copy -Recurse $src $dst -ErrorAction SilentlyContinue
        return ""
    }
    Catch [System.Management.Automation.RuntimeException]
    {
        return "File $src - $($Error[0].Exception.Message)"
    }
    Catch [System.InvalidCastException]
    {
        return "Copying $src failed - $($Error[0].Exception.Message)"
    }
    Catch
    {
        return "File $src - $($Error[0].Exception.Message)"
    }
}


foreach ($i in $installinfo.InstallationInfo) {

    #Print messages
    if ($i.msg) {
        echo "$($i.msg)..."
    }

    #One directory per group
    $dst = "$C2WINSTALL_INPUT\$($i.dir)"

    #Loop through files
    foreach ($f in $i.files) {
        $src = "$CALLIGRA_INST\$f"
        $errstr = copy-newer $src $dst
        if ( $errstr ) {
            if ($Error[0].Exception.GetType() -eq [System.IO.IOException]) {
                echo "INFO: file $dst already exists."
            }
            elseif ($i.optional) {
                echo $errstr
            }
            else {
                Write-Warning $errstr
            }
        }
    }
}


# Had to change:
# share\kde4\services\basicflakesplugin.desktop, share\kde4\services
#


# & $C2WINSTALL_INPUT\..\c2winstaller\res\package\env.bat $C2WINSTALL_INPUT
