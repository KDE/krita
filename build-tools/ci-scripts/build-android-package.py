#!/bin/python3

import os
import re
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

baseWorkDirectoryPath = os.getcwd()
if 'KDECI_WORKDIR_PATH' in os.environ:
    baseWorkDirectoryPath = os.path.join(os.environ['KDECI_WORKDIR_PATH'], 'krita')

buildPath = os.path.abspath(os.path.join(baseWorkDirectoryPath, '_build'))
depsPath = os.path.abspath(os.environ.get('KDECI_SHARED_INSTALL_PATH', os.path.join(baseWorkDirectoryPath, '_install')))
packagingFolder = os.path.join(baseWorkDirectoryPath, '_packaging')

buildEnvironment = dict(os.environ)
buildEnvironment['ANDROID_ABI'] = os.environ['KDECI_ANDROID_ABI']
buildEnvironment['KRITA_INSTALL_PREFIX'] = depsPath

# Gradle 8.13 / AGP 8.12 do not support Java 25 (class file 69). Use JDK 17 or 21 with javac.
def _find_gradle_jdk():
    jdk_bases = ['/usr/lib/jvm', '/usr/lib64/jvm']
    prefer = ['java-21-openjdk', 'java-17-openjdk', 'java-21', 'java-17']  # prefer these names first
    found = []
    for base in jdk_bases:
        if not os.path.isdir(base):
            continue
        for name in sorted(os.listdir(base)):
            jdk_home = os.path.join(base, name)
            if not os.path.isdir(jdk_home):
                continue
            java_exe = os.path.join(jdk_home, 'bin', 'java')
            javac_exe = os.path.join(jdk_home, 'bin', 'javac')
            if not (os.path.isfile(javac_exe) and os.access(javac_exe, os.X_OK)):
                continue
            if not (os.path.isfile(java_exe) and os.access(java_exe, os.X_OK)):
                continue
            try:
                out = subprocess.check_output([java_exe, '-version'], stderr=subprocess.STDOUT, timeout=5).decode()
                if re.search(r'version "[^"]*"(17|18|19|20|21)\.', out):
                    found.append((name in prefer, jdk_home))
            except Exception:
                pass
    if not found:
        return None
    found.sort(key=lambda x: (not x[0], x[1]))
    return found[0][1]

def _java_version_ok(java_home):
    if not java_home or not os.path.isfile(os.path.join(java_home, 'bin', 'javac')):
        return False
    try:
        out = subprocess.check_output([os.path.join(java_home, 'bin', 'java'), '-version'], stderr=subprocess.STDOUT, timeout=5).decode()
        return bool(re.search(r'version "[^"]*"(17|18|19|20|21)\.', out))
    except Exception:
        return False

_current_java = buildEnvironment.get('JAVA_HOME') if _java_version_ok(buildEnvironment.get('JAVA_HOME')) else None
if not _current_java:
    _gradle_jdk = _find_gradle_jdk()
    if _gradle_jdk:
        buildEnvironment['JAVA_HOME'] = _gradle_jdk
        print('## Using JAVA_HOME={} for Gradle (JDK 17/21)'.format(_gradle_jdk))
    else:
        print('## ERROR: Gradle 8.13 requires JDK 17 or 21 (with javac). Current Java is too new (e.g. 25) or no suitable JDK found.')
        print('## Install a full JDK and set JAVA_HOME before running this script:')
        print('##   Fedora/RHEL: sudo dnf install java-21-openjdk-devel')
        print('##   Then: export JAVA_HOME=/usr/lib/jvm/java-21-openjdk')
        print('##   Or: export JAVA_HOME=/usr/lib64/jvm/java-21-openjdk')
        sys.exit(1)

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
    xmlFolder = os.path.join(artifactsFolder, 'res/values/')

    if os.path.isdir(artifactsFolder):
        print("## WARNING: artifacts folder already exists, removing {}".format(artifactsFolder))
        shutil.rmtree(artifactsFolder)

    os.makedirs(artifactsFolder)

    if arguments.skip_common_artifacts:
        shutil.move(os.path.join(repackagePath, 'libs', os.environ['KDECI_ANDROID_ABI']),
                    os.path.join(artifactsFolder, 'libs', os.environ['KDECI_ANDROID_ABI']))
        os.makedirs(xmlFolder)
        shutil.move(os.path.join(repackagePath, 'res/values/libs.xml'),
                    os.path.join(xmlFolder, f'libs-{os.environ["KDECI_ANDROID_ABI"]}.xml'))
    else:
        folders = ['libs',
                   'res',
                   'res-src',
                   'gradle',
                   'assets',
                   'src']

        for folder in folders:
            shutil.move(os.path.join(repackagePath, folder),
                        os.path.join(artifactsFolder, folder))

        files = ['AndroidManifest.xml',
                 'proguard-rules.pro',
                 'build.gradle',
                 'gradlew',
                 'gradle.properties',
                 'local.properties',]

        for file in files:
            shutil.move(os.path.join(repackagePath, file),
                        os.path.join(artifactsFolder, file))

        # We then finally copy the file at the end so we don't conflict with previous copy res procedure.
        shutil.move(os.path.join(artifactsFolder, 'res/values/libs.xml'),
                    os.path.join(xmlFolder, f'libs-{os.environ["KDECI_ANDROID_ABI"]}.xml'))

        # Move the translation folder that is used during the bundle build.
        #
        # During normal package builds this folder is accessed via qt5AndroidDir,
        # which stores an absolute path to _install directory. After moving to the
        # bundler node, _install folder is not available anymore, hence we should
        # copy it manually
        shutil.move(os.path.join(depsPath, 'src', 'android', 'java'),
                    os.path.join(artifactsFolder, 'extra-bundle-deps', 'android', 'java'))
