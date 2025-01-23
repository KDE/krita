#!/bin/python3

import os
import sys
import subprocess
import argparse
import shutil
import glob

supportedPlatforms = ['linux', 'windows', 'macos-universal', 'android-x86_64', 'android-arm64-v8a', 'android-armeabi-v7a']

# Capture our command line parameters
parser = argparse.ArgumentParser(description='A script for building Krita Windows package on CI')
parser.add_argument('--platform', type=str, choices=supportedPlatforms, required=True, help='The platform pacakge is built for')
parser.add_argument('--folder', type=str, required=True, help='The folder where to search for packages')
arguments = parser.parse_args()

skipUploadingNightlyPackages = os.environ.get('KRITACI_SKIP_UPLOAD_NIGHTLY_PACKAGE', 'False')

if skipUploadingNightlyPackages.lower() in ['true', '1', 't', 'y', 'yes']:
    print('## KRITACI_SKIP_UPLOAD_NIGHTLY_PACKAGE is set. Skiping uploading nightly packages...')
    sys.exit(0)

if not 'KRITACI_PUBLISH_BUILD_CONFIG' in os.environ:
    print('## ERROR: KRITACI_PUBLISH_BUILD_CONFIG is not defined')
    sys.exit(1)
elif not os.path.exists(os.environ['KRITACI_PUBLISH_BUILD_CONFIG']):
    print('## ERROR: the file in KRITACI_PUBLISH_BUILD_CONFIG does not exist: {}'
          .format(os.environ['KRITACI_PUBLISH_BUILD_CONFIG']))
    sys.exit(1)

sourcePath = os.path.abspath(arguments.folder)

patterns = []

if arguments.platform == 'windows':
    patterns = ['*.zip', '*.exe', '*.msix']
elif arguments.platform == 'linux':
    patterns = ['*.AppImage', '*.zsync']
    pass
elif arguments.platform == 'macos-universal':
    print('## WARNING: check the pattern for artifacts on macOS!')
    patterns = ['*.dmg']
    pass
elif arguments.platform.startswith('android'):
    patterns = ['*.apk', '*.aab']
    pass

patterns.extend(['*.md5', '*.sha256'])

print ('## Using glob \'{}\' to locate packages at {}'.format(', '.join(patterns), sourcePath))

packagesFolder = os.path.join(os.getcwd(), '.kde-ci-packages')
os.makedirs(packagesFolder)

for pattern in patterns:
    for package in glob.glob(os.path.join(sourcePath, pattern)):
        print( "## Found a package file: {}".format(package))
        shutil.move(package, packagesFolder)

commandToRun = ' '.join([sys.executable,
                         os.path.join('ci-notary-service', 'publishbuild.py'),
                         '--config', os.environ['KRITACI_PUBLISH_BUILD_CONFIG'],
                         '--platform', arguments.platform,
                         packagesFolder])
try:
    print( "## RUNNING: " + commandToRun )
    subprocess.check_call( commandToRun, stdout=sys.stdout, stderr=sys.stderr,
                          shell=True)
except Exception:
    print("## Failed to upload packages")
    sys.exit(1)

