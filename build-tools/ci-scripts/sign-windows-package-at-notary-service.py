import argparse
import os.path
from os import environ
import subprocess
import sys

# command-line args parsing
parser = argparse.ArgumentParser()
parser.add_argument("pkg_root", action='store', help="Specify the package root")
args = parser.parse_args()
pkg_root = args.pkg_root

print(f"Signing binaries in {pkg_root}")
if not os.path.isdir(pkg_root):
    print(f"ERROR: No packaging dir {pkg_root}")
    exit(1)

KRITACI_WINDOWS_SIGN_CONFIG = environ.get('KRITACI_WINDOWS_SIGN_CONFIG')
if not KRITACI_WINDOWS_SIGN_CONFIG:
    print("ERROR: %KRITACI_WINDOWS_SIGN_CONFIG% not set")
    exit(1)
if not os.path.isfile(KRITACI_WINDOWS_SIGN_CONFIG):
    print(f"ERROR: No signing config file found: {KRITACI_WINDOWS_SIGN_CONFIG}")
    exit(1)

with open("files-to-sign.txt", 'w') as toSign:
    with os.scandir(pkg_root) as pkg_files:
        for entry in pkg_files:
            if entry.is_file() and entry.name.endswith(('.exe', '.com', '.dll', '.pyd')):
                print(entry.path, file=toSign)

    commandToRun = "python.exe -u ci-notary-service/signwindowsbinaries.py --config %KRITACI_WINDOWS_SIGN_CONFIG% --files-from files-to-sign.txt"
    subprocess.check_call(commandToRun, stdout=sys.stdout, stderr=sys.stderr, shell=True )
