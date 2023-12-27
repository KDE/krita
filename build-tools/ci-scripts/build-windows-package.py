#!/bin/python3

import os
import sys
import subprocess
import argparse

# Capture our command line parameters
parser = argparse.ArgumentParser(description='A script for building Krita Windows package on CI')
parser.add_argument('--skip-debug-package', default=False, action='store_true')
arguments = parser.parse_args()

buildPath = os.path.abspath('_build')
depsPath = os.path.abspath('_install')

if arguments.skip_debug_package:
    os.environ['KRITA_SKIP_DEBUG_PACKAGE'] = '1'

kritaVersionString = ''

with open(os.path.join(buildPath, "libs/version/kritaversion.h"), "r") as fp:
    for line in fp:
        if line.strip().startswith('#define KRITA_VERSION_STRING'):
            kritaVersionString = line.split()[-1].strip('\"')
            print ('krita version: {}'.format(kritaVersionString))
            break

commandToRun = ' '.join(['packaging\windows\package-complete.cmd',
                         '--no-interactive',
                         '--package-name', 'krita-{}-{}'.format(kritaVersionString, os.environ['CI_COMMIT_SHORT_SHA']),
                         '--src-dir',  os.getcwd(),
                         '--deps-install-dir', depsPath,
                         '--krita-install-dir', depsPath])

# Run the command
try:
    print( "## RUNNING: " + commandToRun )
    subprocess.check_call( commandToRun, stdout=sys.stdout, stderr=sys.stderr, shell=True )
except Exception:
    print("## Failed to build the appimage")
    sys.exit(1)
