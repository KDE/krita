#!/bin/python3

import os
import sys
import subprocess
import argparse
import shutil
import glob

# Capture our command line parameters
parser = argparse.ArgumentParser(description='A script for building Krita Android package on CI')
parser.add_argument('--package-type', type=str, choices=['debug', 'release', 'nightly'], default = 'debug', help='Type of a package to build')
parser.add_argument('--archive-artifacts', default=False, action='store_true', help='Create an folder with artifacts')
parser.add_argument('--skip-common-artifacts', default=False, action='store_true', help='Skip artifacts that are the same on all architectures')
arguments = parser.parse_args()

if not 'KDECI_ANDROID_ABI' in os.environ:
    print('## ERROR: KDECI_ANDROID_ABI is not set!')
    sys.exit(1)

if 'KRITACI_ARCHIVE_ARTIFACTS' in os.environ:
    arguments.archive_artifacts = (os.environ['KRITACI_ARCHIVE_ARTIFACTS'].lower() in ['true', '1', 't', 'y', 'yes'])
    print ('## Overriding --archive-artifacts from environment: {}'.format(arguments.archive_artifacts))

if 'KRITACI_SKIP_COMMON_ARTIFACTS' in os.environ:
    arguments.skip_common_artifacts = (os.environ['KRITACI_SKIP_COMMON_ARTIFACTS'].lower() in ['true', '1', 't', 'y', 'yes'])
    print ('## Overriding --skip-common-artifacts from environment: {}'.format(arguments.skip_common_artifacts))

if arguments.skip_common_artifacts and not arguments.archive_artifacts:
    print('## ERROR: --skip-common-artifacts cannot be used without --archive-artifacts')
    sys.exit(1)

if 'KRITACI_ANDROID_PACKAGE_TYPE' in os.environ:
    arguments.package_type = os.environ['KRITACI_ANDROID_PACKAGE_TYPE']
    if not arguments.package_type in ['debug', 'release', 'nightly']:
        print('## ERROR: incorrect package type provided via KRITACI_ANDROID_PACKAGE_TYPE: {}'.format(arguments.package_type))
        sys.exit(1)
    print ('## Overriding --package-type from environment: {}'.format(arguments.package_type))

buildPath = os.path.abspath('_build')
depsPath = os.path.abspath('_install')
srcPath = os.path.abspath(os.getcwd())
packagingFolder = os.path.join(srcPath, '_packaging')

buildEnvironment = dict(os.environ)
buildEnvironment['ANDROID_ABI'] = os.environ['KDECI_ANDROID_ABI']
buildEnvironment['KRITA_INSTALL_PREFIX'] = depsPath

commandToRun = 'cmake --build . --target create-apk'
try:
    print( "## RUNNING: " + commandToRun )
    subprocess.check_call( commandToRun, stdout=sys.stdout, stderr=sys.stderr, shell=True, cwd=buildPath, env=buildEnvironment )
except Exception:
    print("## Failed to build apk")
    sys.exit(1)

repackagePath = os.path.join(buildPath, 'krita_build_apk')
if arguments.package_type != 'debug':

    commandToRun = './gradlew clean'
    try:
        print( "## RUNNING: " + commandToRun )
        subprocess.check_call( commandToRun, stdout=sys.stdout, stderr=sys.stderr,
                              shell=True, cwd=repackagePath, env=buildEnvironment )
    except Exception:
        print("## Failed to clean package directory")
        sys.exit(1)

    if arguments.package_type == 'release':
        commandToRun = './gradlew assembleRelease'
    elif arguments.package_type == 'nightly':
        commandToRun = './gradlew assembleNightly'

    try:
        print( "## RUNNING: " + commandToRun )
        subprocess.check_call( commandToRun, stdout=sys.stdout, stderr=sys.stderr,
                              shell=True, cwd=repackagePath, env=buildEnvironment )
    except Exception:
        print("## Failed to repackage {} package".format(arguments.package_type))
        sys.exit(1)



if os.path.isdir(packagingFolder):
    print("## WARNING: packaging folder already exists, removing {}".format(packagingFolder))
    shutil.rmtree(packagingFolder)

os.makedirs(packagingFolder)

for package in glob.glob(os.path.join(repackagePath, 'build', 'outputs', 'apk', '**', '*.apk'), recursive=True):
    print( "## Found a package file: {}".format(package))
    shutil.move(package, packagingFolder)

if arguments.archive_artifacts:
    artifactsFolder = os.path.join(packagingFolder, 'krita_build_apk')

    if os.path.isdir(artifactsFolder):
        print("## WARNING: artifacts folder already exists, removing {}".format(artifactsFolder))
        shutil.rmtree(artifactsFolder)

    os.makedirs(artifactsFolder)

    if arguments.skip_common_artifacts:
        shutil.copytree(os.path.join(repackagePath, 'libs', os.environ['KDECI_ANDROID_ABI']),
                        os.path.join(artifactsFolder, 'libs', os.environ['KDECI_ANDROID_ABI']))
    else:
        folders = ['libs',
                   'res',
                   'res-src',
                   'gradle',
                   'assets',
                   'src']

        for folder in folders:
            shutil.copytree(os.path.join(repackagePath, folder),
                            os.path.join(artifactsFolder, folder))

        files = ['AndroidManifest.xml',
                 'proguard-rules.pro',
                 'build.gradle',
                 'gradlew',
                 'gradle.properties',
                 'local.properties',]

        for file in files:
            shutil.copy2(os.path.join(repackagePath, file),
                         os.path.join(artifactsFolder, file))
