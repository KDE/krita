# SPDX-FileCopyrightText: 2026 Dmitry Kazakov <dimula73@gmail.com>
# SPDX-License-Identifier: GPL-2.0-or-later

import argparse
import os
import subprocess
import warnings
import sys

has_pefile = False

try:
    import pefile
    has_pefile = True
except:
    pass


glob_patterns = ['*.dll', '*.exe', '*.com', '*.pyd']

def find_files(directory):
    for root, _, files in os.walk(directory):
        for pattern in glob_patterns:
            for fname in files:
                if fname.lower().endswith(pattern[1:]):
                    yield os.path.join(root, fname)

def has_debug_section(objdumpOutput):
    for line in objdumpOutput.splitlines():
        if '.debug_' in line:
            return True
    return False

def has_certificate_entry(filePath):
    # NOTE: we do **not** verify the signature itself here,
    # we just check if the entry is present in the PE-structure
    pe = pefile.PE(filePath, fast_load=True)
    address = pefile.DIRECTORY_ENTRY["IMAGE_DIRECTORY_ENTRY_SECURITY"]
    return pe.OPTIONAL_HEADER.DATA_DIRECTORY[address].Size > 0

parser = argparse.ArgumentParser(description=f'Searches for {', '.join(glob_patterns)} files with DEBUG section present.')
parser.add_argument('-d', '--directory', default=None, help='Directory to search (default: current directory)')
parser.add_argument('-f', '--file', default=None, help='Executable file to check')
if has_pefile:
    parser.add_argument('-s', '--signature', action='store_true', default=False, help='Verify that all modules in the directory has signature entry (no signature validation happens)')
args = parser.parse_args()

if args.directory is None and args.file is None:
    args.directory = os.getcwd()

OBJDUMP = False
for arg in ("objdump", "llvm-objdump"):
    commandToRun = f"{arg} --version"
    ret = subprocess.call(commandToRun, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, shell=True)
    if ret != 1:
        OBJDUMP = arg
if not OBJDUMP:
    warnings.warn("ERROR: objdump is not working.")
    sys.exit(1)

anyLibsWithDebugFound = False
anyUnsignedFound = False

def verify_one_file(file):
    global anyLibsWithDebugFound
    global anyUnsignedFound
    try:
        if has_pefile and args.signature:
            if not has_certificate_entry(file):
              anyUnsignedFound = True
              print(f"File is not signed: {file}")
        result = subprocess.run([OBJDUMP, '-h', file], capture_output=True, text=True, check=True)
        if has_debug_section(result.stdout):
            anyLibsWithDebugFound = True
            print(result.stdout)
    except Exception as e:
        warnings.warn(f"ERROR: Failed to parse: {file}")
        raise e

if args.directory is not None:
    for file in find_files(args.directory):
        verify_one_file(file)

if args.file is not None:
    verify_one_file(args.file)

if anyLibsWithDebugFound or anyUnsignedFound:
    sys.exit(2)