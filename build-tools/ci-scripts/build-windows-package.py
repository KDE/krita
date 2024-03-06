#!/bin/python3

import os
import sys
import subprocess
import argparse
import shutil
import copy

# Capture our command line parameters
parser = argparse.ArgumentParser(description='A script for building Krita Windows package on CI')
parser.add_argument('--skip-debug-package', default=False, action='store_true')
parser.add_argument('--build-installers', default=False, action='store_true')
arguments = parser.parse_args()

if 'KRITACI_SKIP_DEBUG_PACKAGE' in os.environ:
    arguments.skip_debug_package = (os.environ['KRITACI_SKIP_DEBUG_PACKAGE'].lower() in ['true', '1', 't', 'y', 'yes'])
    print ('## Overriding --skip-debug-package from environment: {}'.format(arguments.skip_debug_package))

if 'KRITACI_BUILD_INSTALLERS' in os.environ:
    arguments.build_installers = (os.environ['KRITACI_BUILD_INSTALLERS'].lower() in ['true', '1', 't', 'y', 'yes'])
    print ('## Overriding --build-installers from environment: {}'.format(arguments.build_installers))

buildPath = os.path.abspath('_build')
depsPath = os.path.abspath('_install')
srcPath = os.path.abspath(os.getcwd())

if arguments.skip_debug_package:
    os.environ['KRITA_SKIP_DEBUG_PACKAGE'] = '1'

kritaVersionString = ''

with open(os.path.join(buildPath, "libs/version/kritaversion.h"), "r") as fp:
    for line in fp:
        if line.strip().startswith('#define KRITA_VERSION_STRING'):
            kritaVersionString = line.split()[-1].strip('\"')
            print ('krita version: {}'.format(kritaVersionString))
            break

packageName = 'krita-{}-{}'.format(kritaVersionString, os.environ['CI_COMMIT_SHORT_SHA'])
hookFile = os.path.join(srcPath, 'build-tools', 'ci-scripts', 'sign-windows-package-at-notary-service.cmd')

commandToRun = ' '.join(['cmd.exe /c',
                         'packaging\windows\package-complete.cmd',
                         '--no-interactive',
                         '--package-name', packageName,
                         '--src-dir',  srcPath,
                         '--deps-install-dir', depsPath,
                         '--krita-install-dir', depsPath,
                         '--pre-zip-hook', '\"{}\"'.format(os.path.join(srcPath, 'build-tools', 'ci-scripts', 'sign-windows-package-at-notary-service.cmd'))])

# Run the command
try:
    print( "## RUNNING: " + commandToRun )
    subprocess.check_call( commandToRun, stdout=sys.stdout, stderr=sys.stderr, shell=True )
except Exception:
    print("## Failed to build zip package")
    sys.exit(1)

if arguments.build_installers:

    installerFileName = '{}-setup.exe'.format(packageName)
    packageFolder = os.path.join(srcPath, packageName)

    installerFolder = os.path.join(os.getcwd(), '_installer')
    os.makedirs(installerFolder)

    commandToRun = ' '.join(['cmake',
                            '-DREMOVE_DEBUG=ON',
                            '-DOUTPUT_FILEPATH={}'.format(installerFileName),
                            '-DKRITA_PACKAGE_ROOT={}'.format(packageFolder),
                            '-P', os.path.join(depsPath, 'MakeinstallerNsis.cmake')
                            ])

    # Run the command
    try:
        print( "## RUNNING: " + commandToRun )
        subprocess.check_call( commandToRun, stdout=sys.stdout, stderr=sys.stderr, shell=True,
                              cwd=installerFolder )
    except Exception:
        print("## Failed to build the installer")
        sys.exit(1)

    shutil.move(os.path.join(installerFolder, 'krita-nsis', installerFileName), os.getcwd())

    commandToRun = ' '.join([sys.executable,
                            os.path.join('ci-notary-service', 'signwindowsbinaries.py'),
                            '--config', os.path.join('krita-deps-management', 'ci-utilities', 'signing', 'signwindowsbinaries.ini'),
                            installerFileName
                            ])

    # Run the command
    try:
        print( "## RUNNING: " + commandToRun )
        subprocess.check_call( commandToRun, stdout=sys.stdout, stderr=sys.stderr, shell=True )
    except Exception:
        print("## Failed to sign the installer")
        sys.exit(1)

    msixFolder = os.path.join(os.getcwd(), '_msix')
    os.makedirs(msixFolder)

    msixEnvironment = copy.deepcopy(dict(os.environ))

    msixEnvironment['KRITA_DIR'] = packageFolder
    msixEnvironment['KRITA_SHELLEX'] = os.path.join(srcPath, '_installer', 'krita-nsis')
    msixEnvironment['OUTPUT_DIR'] = msixFolder

    # build_msix.cmd populates this directory itself
    if os.path.exists(os.path.join(packageFolder, 'shellex')):
        shutil.rmtree(os.path.join(packageFolder, 'shellex'))

    commandToRun = ' '.join(['cmd.exe /c',
                            os.path.join(depsPath, 'krita-msix', 'build_msix.cmd')
                            ])

    # Run the command
    try:
        print( "## RUNNING: " + commandToRun )
        subprocess.check_call( commandToRun, stdout=sys.stdout, stderr=sys.stderr, shell=True,
                               env=msixEnvironment, cwd=msixFolder )
    except Exception:
        print("## Failed to build the MSIX package")
        sys.exit(1)

    shutil.move(os.path.join(msixFolder, 'krita.msix'),
                os.path.join(os.getcwd(), '{}-unsigned.msix'.format(packageName)))
