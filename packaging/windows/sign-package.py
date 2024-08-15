import argparse
from os import environ
import os.path
import glob
import subprocess
import sys

# command-line args parsing
parser = argparse.ArgumentParser()
parser.add_argument("pkg_root", action='store', help="Specify the package root", required=True)
args = parser.parse_args()
pkg_root = args.pkg_root

if not environ.get('SIGNTOOL'):
    # Find Windows SDK for signtool.exe
    if (not environ.get('WindowsSdkDir')) and environ.get('ProgramFiles(x86)'):
        environ['WindowsSdkDir'] = fr"{environ['ProgramFiles(x86)']}\Windows Kits\10"
    if os.path.isfile(environ.get('WindowsSdkDir')):
        sdkDir = environ["WindowsSdkDir"]
        delims = glob.glob(fr"{sdkDir}\bin\10.*")
        for dir in delims:
            if os.path.isfile(sdkDir + fr"bin\{dir}\x64\signtool.exe"):
                environ['SIGNTOOL'] = fr"{sdkDir}\{dir}\bin\x86\signtool.exe"
        if not environ.get('SIGNTOOL'):
            if os.path.isfile(fr"{sdkDir}\bin\x64\signtool.exe"):
                environ['SIGNTOOL'] = fr"{sdkDir}\bin\x86\signtool.exe"
    if not environ.get('SIGNTOOL'):
        print("ERROR: signtool not found")
        exit(1)


if not environ.get('SIGNTOOL_SIGN_FLAGS'):
    print("ERROR: Please set environment variable SIGNTOOL_SIGN_FLAGS")
    exit(1)
    # This is what I used for testing:
    # set "SIGNTOOL_SIGN_FLAGS=/f "C:\Users\Alvin\MySPC.pfx" /t http://timestamp.verisign.com/scripts/timstamp.dll"

print(f"Signing binaries in {pkg_root}")
if not os.path.isdir(pkg_root):
    print(f"ERROR: No packaging dir {pkg_root}")
    exit(1)

with os.scandir(pkg_root) as pkg_files:
    for entry in pkg_files:
        if entry.is_file() and entry.name.endswith(('.exe', '.com', '.dll', '.pyd')):
            # Check for existing signature
            commandToRun = f'"%SIGNTOOL%" verify /q /pa "{entry.name}"'
            try:
                subprocess.check_call(commandToRun, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, shell=True)
            except subprocess.CalledProcessError as status:
                if status.returncode == 1:
                    print(f"Signing {entry.path}")
                    try:
                        commandToRun = f'"%SIGNTOOL%" sign %SIGNTOOL_SIGN_FLAGS% "{entry.path}"'
                        subprocess.check_call(commandToRun, stdout=sys.stdout, stderr=sys.stderr, shell=True)
                    except subprocess.CalledProcessError as status:
                        print(f"ERROR: Got exit code {status.returncode} from signtool!")
                        exit(1)
            else:
                print(f"Not signing {entry.path} - file already signed")
