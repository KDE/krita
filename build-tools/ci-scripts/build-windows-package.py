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
parser.add_argument('--release-package-naming', default=False, action='store_true')
arguments = parser.parse_args()

if 'KRITACI_SKIP_DEBUG_PACKAGE' in os.environ:
    arguments.skip_debug_package = (os.environ['KRITACI_SKIP_DEBUG_PACKAGE'].lower() in ['true', '1', 't', 'y', 'yes'])
    print ('## Overriding --skip-debug-package from environment: {}'.format(arguments.skip_debug_package))

if 'KRITACI_BUILD_INSTALLERS' in os.environ:
    arguments.build_installers = (os.environ['KRITACI_BUILD_INSTALLERS'].lower() in ['true', '1', 't', 'y', 'yes'])
    print ('## Overriding --build-installers from environment: {}'.format(arguments.build_installers))

if 'KRITACI_RELEASE_PACKAGE_NAMING' in os.environ:
    arguments.release_package_naming = (os.environ['KRITACI_RELEASE_PACKAGE_NAMING'].lower() in ['true', '1', 't', 'y', 'yes'])
    print ('## Overriding --release-package-naming from environment: {}'.format(arguments.release_package_naming))

signPackages = True

if not 'KRITACI_WINDOWS_SIGN_CONFIG' in os.environ:
    print('## WARNING: KRITACI_WINDOWS_SIGN_CONFIG is not defined, signing will be skipped')
    signPackages = False
elif not os.path.exists(os.environ['KRITACI_WINDOWS_SIGN_CONFIG']):
    print('## WARNING: the file in KRITACI_WINDOWS_SIGN_CONFIG does not exist, signing will be skipped: {}'
          .format(os.environ['KRITACI_WINDOWS_SIGN_CONFIG']))
    signPackages = False

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


unstablePackageSuffix = '-{}'.format(os.environ['CI_COMMIT_SHORT_SHA']) \
    if not arguments.release_package_naming else ''

packageName = 'krita-x64-{}{}'.format(kritaVersionString, unstablePackageSuffix)
hookFile = os.path.join(srcPath, 'build-tools', 'ci-scripts', 'sign-windows-package-at-notary-service.py')

commandToRun = ' '.join([sys.executable,
                         'packaging\windows\package-complete.py',
                         '--no-interactive',
                         '--package-name', packageName,
                         '--src-dir',  srcPath,
                         '--deps-install-dir', depsPath,
                         '--krita-install-dir', depsPath,
                         f'--pre-zip-hook \"{hookFile}\"' if signPackages else ''
                        ])

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

    if signPackages:
        commandToRun = ' '.join([sys.executable,
                                os.path.join('ci-notary-service', 'signwindowsbinaries.py'),
                                '--config', os.environ['KRITACI_WINDOWS_SIGN_CONFIG'],
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

    # build_msix.py populates this directory itself
    if os.path.exists(os.path.join(packageFolder, 'shellex')):
        shutil.rmtree(os.path.join(packageFolder, 'shellex'))

    commandToRun = ' '.join([sys.executable,
                            os.path.join(depsPath, 'krita-msix', 'build_msix.py')
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
