#!/bin/python3

import requests
import tarfile
import shutil
import os
import json
import sys
import subprocess

# Converts a path to a relative one, to allow for it to be passed to os.path.join
# This is primarily relevant on Windows, where full paths have the drive letter, and thus can be simply joined together as you can on Unix systems
def makePathRelative(path):
    # If we're on Windows, chop the drive letter off...
    if sys.platform == "win32":
        return path[3:]

    # Otherwise we just drop the starting slash off
    return path[1:]

packagingPath = os.path.abspath('_packaging')
appdirPath = os.path.join(packagingPath, 'krita.appdir')
downloadsPath = os.path.join(packagingPath, 'download')
buildPath = os.path.abspath('_build')
depsPath = os.path.abspath('_install')
stagingRoot = os.path.join('_staging', makePathRelative(depsPath))

if os.path.isdir(appdirPath):
    shutil.rmtree(appdirPath)

os.makedirs(os.path.join(appdirPath, 'usr'))

if not os.path.isdir(downloadsPath):
    os.makedirs(downloadsPath)

filesToMove = os.listdir(stagingRoot)
for file in filesToMove:
    shutil.move(os.path.join(stagingRoot, file), os.path.join(appdirPath, 'usr'))

os.environ.putenv('KRITA_APPDIR_PATH', appdirPath)
os.environ.putenv('KRITA_DOWNLOADS_PATH', downloadsPath)
os.environ.putenv('KRITA_BUILD_PATH', buildPath)
os.environ.putenv('KRITA_DEPS_PATH', depsPath)

commandToRun = ' '.join(['./packaging/linux/appimage/build-image.sh', packagingPath, os.path.abspath('.')])

# Run the command
try:
    print( "## RUNNING: " + commandToRun )
    subprocess.check_call( commandToRun, stdout=sys.stdout, stderr=sys.stderr, shell=True )
except Exception:
    print("## Failed to build the appimage")
    sys.exit(1)

# cleanup temporary artifacts that we don't want to upload
shutil.rmtree(appdirPath)
shutil.rmtree(downloadsPath)
