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
    sys.exit(1)

KRITACI_WINDOWS_SIGN_CONFIG = environ.get('KRITACI_WINDOWS_SIGN_CONFIG')
if not KRITACI_WINDOWS_SIGN_CONFIG:
    print("ERROR: %KRITACI_WINDOWS_SIGN_CONFIG% not set")
    sys.exit(1)
if not os.path.isfile(KRITACI_WINDOWS_SIGN_CONFIG):
    print(f"ERROR: No signing config file found: {KRITACI_WINDOWS_SIGN_CONFIG}")
    sys.exit(1)

with open("files-to-sign.txt", 'w') as toSign:
    for rootPath, dirs, files in os.walk(pkg_root):
        for fileName in files:
            if fileName.endswith(('.exe', '.com', '.dll', '.pyd')):
                filePath = os.path.join(rootPath, fileName)
                print(filePath, file=toSign)

commandToRun = f"{sys.executable} -u ci-notary-service/signwindowsbinaries.py --config {KRITACI_WINDOWS_SIGN_CONFIG} --files-from files-to-sign.txt"
subprocess.check_call(commandToRun, stdout=sys.stdout, stderr=sys.stderr, shell=True )
