#!/bin/python3

import os
import sys
import subprocess
import argparse
import shutil
import glob

# Capture our command line parameters
parser = argparse.ArgumentParser(description='A script for building Krita Android AppBundle package on CI')
parser.add_argument('--package-type', type=str, choices=['debug', 'release', 'nightly'], default = 'debug', help='Type of a package to build')
arguments = parser.parse_args()

if 'KRITACI_ANDROID_PACKAGE_TYPE' in os.environ:
    arguments.package_type = os.environ['KRITACI_ANDROID_PACKAGE_TYPE']
    if not arguments.package_type in ['debug', 'release', 'nightly']:
        print('## ERROR: incorrect package type provided via KRITACI_ANDROID_PACKAGE_TYPE: {}'.format(arguments.package_type))
        sys.exit(1)
    print ('## Overriding --package-type from environment: {}'.format(arguments.package_type))

currentFileDirectoryPath = os.path.dirname(os.path.abspath(__file__))
baseWorkDirectoryPath = os.getcwd()
if 'KDECI_WORKDIR_PATH' in os.environ:
    baseWorkDirectoryPath = os.path.join(os.environ['KDECI_WORKDIR_PATH'], 'krita')

packagingFolder = os.path.join(baseWorkDirectoryPath, '_packaging')
artifactsFolder = os.path.join(packagingFolder, 'krita_build_apk')

buildEnvironment = dict(os.environ)

buildEnvironment['KRITA_BUILD_APPBUNDLE'] = '1'
buildEnvironment['APK_PATH'] = artifactsFolder
buildEnvironment['KRITA_INSTALL_PREFIX'] = '.xxx' # just a fake path to trigger an error if used (if shouldn't be used for aab)

if arguments.package_type == 'release':
    unstablePackageSuffix = ''
else:
    shortSha = os.environ.get('CI_COMMIT_SHORT_SHA')
    if shortSha is None:
        print('## CI_COMMIT_SHORT_SHA not set, attempting to retrieve it through git')
        scriptDir = os.path.dirname(os.path.abspath(__file__))
        shortSha = subprocess.check_output(['git', 'log', '--pretty=%h', '-n', '1'], cwd=scriptDir).decode().strip()

    if not shortSha:
        print('## ERROR: no unstable package suffix, please set CI_COMMIT_SHORT_SHA')
        sys.exit(1)

    unstablePackageSuffix = '-{}'.format(shortSha)

buildEnvironment['KRITA_UNSTABLE_PACKAGE_SUFFIX'] = unstablePackageSuffix

try:
    print( "## MERGING libs.xml" )
    # TODO: if we need x86
    xml_paths = [f'{packagingFolder}/krita_build_apk/res/values/libs-{arch}.xml'
                 for arch in ['arm64-v8a', 'x86_64', 'armeabi-v7a']]
    xml_paths_str = ",".join(xml_paths)
    subprocess.check_call(f'{sys.executable} merge-libs-xml.py -p {xml_paths_str} --output {packagingFolder}/krita_build_apk/res/values/libs.xml',
                          stdout=sys.stdout, stderr=sys.stderr, shell=True, cwd=currentFileDirectoryPath, env=buildEnvironment)
except Exception:
    print("## Failed to merge libs.xml files")
    sys.exit(1)

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
