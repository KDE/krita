#!/usr/bin/python3
import os
import sys
import subprocess
import multiprocessing
import shutil

if 'ANDROID_HOME' in os.environ or 'KDECI_ANDROID_ABI' in os.environ:
    print ('## Skip building GMic plugin for Android...')
    sys.exit(0)

sourcesPath = os.environ.pop('KDECI_SOURCES_DIR')
localCachePath = os.environ.pop('KDECI_CACHE_PATH')
kritaCacheDir = os.path.join(localCachePath, 'krita-deps')
if not os.path.isdir(kritaCacheDir):
    os.makedirs(kritaCacheDir)

buildPath = os.path.join(os.getcwd(), '_build_plugins')
if os.path.isdir(buildPath):
    shutil.rmtree(buildPath)
os.makedirs(buildPath)

cpuCount = int(multiprocessing.cpu_count())
useCcacheForBuilds = os.environ.pop('KDECI_INTERNAL_USE_CCACHE') == 'True'

cmakeCommand = [
    # Run CMake itself
    'cmake',
    '-G Ninja',
    '-DINSTALL_ROOT={}'.format(os.path.join(os.getcwd(), '_install')),
    '-DEXTERNALS_DOWNLOAD_DIR={}'.format(kritaCacheDir),
    '-DCMAKE_BUILD_TYPE={}'.format(os.environ.pop('KDECI_BUILD_TYPE')),
    os.path.join(sourcesPath, '3rdparty_plugins')
]

if useCcacheForBuilds:
    # Then instruct CMake accordingly...
    cmakeCommand.append('-DCMAKE_C_COMPILER_LAUNCHER=ccache')
    cmakeCommand.append('-DCMAKE_CXX_COMPILER_LAUNCHER=ccache')

    # Since we build external projects, we should propagate the 
    # ccache options via the environment variables...
    os.environ['CMAKE_C_COMPILER_LAUNCHER'] = 'ccache'
    os.environ['CMAKE_CXX_COMPILER_LAUNCHER'] = 'ccache'


commandToRun = ' '.join(cmakeCommand)

# Run the CMake command
try:
    print( "## RUNNING: " + commandToRun )
    subprocess.check_call( commandToRun, stdout=sys.stdout, stderr=sys.stderr, shell=True, cwd=buildPath)
except Exception:
    print("## Failed to configure plugins")
    sys.exit(1)

commandToRun = 'cmake --build . --target all --parallel {cpu_count}'.format(cpu_count = cpuCount)

# Run the CMake command
try:
    print( "## RUNNING: " + commandToRun )
    subprocess.check_call( commandToRun, stdout=sys.stdout, stderr=sys.stderr, shell=True, cwd=buildPath)
except Exception:
    print("## Failed to build plugins")
    sys.exit(1)
