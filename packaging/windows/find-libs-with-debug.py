# SPDX-FileCopyrightText: 2026 Dmitry Kazakov <dimula73@gmail.com>
# SPDX-License-Identifier: GPL-2.0-or-later

import argparse
import os
import subprocess
import warnings
import sys
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

parser = argparse.ArgumentParser(description=f'Searches for {', '.join(glob_patterns)} files with DEBUG section present.')
parser.add_argument('-d', '--directory', default=os.getcwd(), help='Directory to search (default: current directory)')
args = parser.parse_args()

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

for file in find_files(args.directory):
    try:
        result = subprocess.run([OBJDUMP, '-h', file], capture_output=True, text=True, check=True)
        if has_debug_section(result.stdout):
            anyLibsWithDebugFound = True
            print(result.stdout)
    except Exception as e:
        warnings.warn(f"ERROR: Failed to parse: {file}")
        raise e

if anyLibsWithDebugFound:
    sys.exit(2)