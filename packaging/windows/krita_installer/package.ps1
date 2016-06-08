#  Copyright (c) 2015 Michael Abrahams <miabraha@gmail.com>
#  package_calligra.ps1
#
#  Copies relevant files from the calligra inst and kderoot folders.
#
#
#  Information about the files to copy is contained in krita-files.json. The
#  directory structure is specified here.
#

#  Safety check:
#  Make sure key variables are defined

if (($env:KRITA_INPUT -eq $null) -or ($env:KRITA_OUTPUT -eq $null)) {
  echo "!!! ENVIRONMENT VARIABLES NOT PROPERLY DEFINED !!!"
  echo "(Did you run env.ps1?)"
  Exit
}

# Create directories for the installer.
mkdir "$env:KRITA_INPUT" -ea SilentlyContinue
mkdir "$env:KRITA_INPUT\bin" -ea SilentlyContinue
mkdir "$env:KRITA_INPUT\etc" -ea SilentlyContinue
mkdir "$env:KRITA_INPUT\lib" -ea SilentlyContinue
mkdir "$env:KRITA_INPUT\lib\kritaplugins" -ea SilentlyContinue
mkdir "$env:KRITA_INPUT\lib\plugins\styles" -ea SilentlyContinue
mkdir "$env:KRITA_INPUT\lib\plugins\imageformats" -ea SilentlyContinue
mkdir "$env:KRITA_INPUT\share" -ea SilentlyContinue
mkdir "$env:KRITA_INPUT\share\appdata" -ea SilentlyContinue
mkdir "$env:KRITA_INPUT\share\applications" -ea SilentlyContinue
mkdir "$env:KRITA_INPUT\share\krita" -ea SilentlyContinue
mkdir "$env:KRITA_INPUT\share\config" -ea SilentlyContinue
mkdir "$env:KRITA_INPUT\etc\dbus-1\" -ea SilentlyContinue
mkdir "$env:KRITA_INPUT\etc\xdg\" -ea SilentlyContinue
mkdir "$env:KRITA_INPUT\share\dbus-1\" -ea SilentlyContinue
mkdir "$env:KRITA_INPUT\share\dbus-1\interfaces" -ea SilentlyContinue
mkdir "$env:KRITA_INPUT\share\dbus-1\services" -ea SilentlyContinue
mkdir "$env:KRITA_INPUT\share\dbus-1\system-services"-ea SilentlyContinue
mkdir "$env:KRITA_INPUT\share\kservices5" -ea SilentlyContinue
mkdir "$env:KRITA_INPUT\share\kservicetypes5" -ea SilentlyContinue

mkdir "$env:KRITA_OUTPUT" -ea SilentlyContinue

# The format of the JSON is as follows.
# Each data item has four elements.
#
# "dir" is the destination directory, relative to $KRITA_INPUT. It is posssible
#       to have several different groups of files copied to the same destination
#       directory. DIR must exist already, created in the "mkdir" list above.
# "files" is a list of files copied into DIR.
# "msg" will be displayed during the copying procedure.
# "comment" is ignored by this script.
$installinfo = ConvertFrom-Json ((Get-Content "krita-files.json") -join "`n")


# Helper function - basically an rsync kinda thing.
# Copy items only if they are newer, return error string on failure.
function copy-newer($src, $dst) {
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
                # Source file is not newer, skip this file
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

    # Print messages from JSON data
    if ($i.msg) {
        echo "$($i.msg)..."
    }

    # One directory per group in JSON data
    $dst = "$env:KRITA_INPUT\$($i.dir)"

    #Loop through files
    foreach ($f in $i.files) {
        $src = "$env:CALLIGRA_INST\$f"
        echo "Copying - $src"
        $errstr = copy-newer $src $dst

        # If we got an error when copying, print something helpful
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
