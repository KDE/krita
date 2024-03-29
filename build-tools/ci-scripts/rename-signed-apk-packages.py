#!/bin/python3

import os
import argparse
import shutil
import glob

supportedPlatforms = ['linux', 'windows', 'macos', 'android-x86_64', 'android-arm64-v8a', 'android-armeabi-v7a']

# Capture our command line parameters
parser = argparse.ArgumentParser(description='A script for building Krita Windows package on CI')
parser.add_argument('--folder', type=str, required=True, help='The folder where to search for packages')
arguments = parser.parse_args()

sourcePath = os.path.abspath(arguments.folder)

for package in glob.glob(os.path.join(sourcePath, '*-unsigned.apk')):
    print( "## Found an allegedly unsigned package file: {}".format(package))
    newName = package.replace('-unsigned.apk', '.apk')
    print( "##     rename: {} -> {}".format(os.path.basename(package), os.path.basename(newName)))
    shutil.move(package, newName)
