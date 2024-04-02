#!/bin/python3

import os
import sys
import subprocess
import argparse
import shutil
import glob

# Capture our command line parameters
parser = argparse.ArgumentParser(description='A script for building Krita Android AppBundle package on CI')
arguments = parser.parse_args()

baseWorkDirectoryPath = os.getcwd()
if 'KDECI_WORKDIR_PATH' in os.environ:
    baseWorkDirectoryPath = os.path.join(os.environ['KDECI_WORKDIR_PATH'], 'krita')

packagingFolder = os.path.join(baseWorkDirectoryPath, '_packaging')
artifactsFolder = os.path.join(packagingFolder, 'krita_build_apk')

buildEnvironment = dict(os.environ)

buildEnvironment['KRITA_BUILD_APPBUNDLE'] = '1'
buildEnvironment['APK_PATH'] = artifactsFolder
buildEnvironment['KRITA_INSTALL_PREFIX'] = '.xxx' # just a fake path to trigger an error if used (if shouldn't be used for aab)

commandToRun = './gradlew bundleRelease'
try:
    print( "## RUNNING: " + commandToRun )
    subprocess.check_call( commandToRun, stdout=sys.stdout, stderr=sys.stderr,
                          shell=True, cwd=artifactsFolder, env=buildEnvironment )
except Exception:
    print("## Failed to build an AppBundle")
    sys.exit(1)

for package in glob.glob(os.path.join(artifactsFolder, 'build', 'outputs', 'bundle', '**', '*.aab'), recursive=True):
    print( "## Found a bundle file: {}".format(package))
    shutil.move(package, packagingFolder)
