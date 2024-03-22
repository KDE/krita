#!/bin/python3

import os
import sys
import subprocess
import argparse

# Capture our command line parameters
parser = argparse.ArgumentParser(description='A script for building Krita Android package on CI')
parser.add_argument('--package-type', type=str, choices=['debug', 'release', 'nightly'], default = 'debug', help='Type of a package to build')
arguments = parser.parse_args()

if not 'KDECI_ANDROID_ABI' in os.environ:
    print('## ERROR: KDECI_ANDROID_ABI is not set!')
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

if arguments.package_type != 'debug':
    repackagePath = os.path.join(buildPath, 'krita_build_apk')

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
