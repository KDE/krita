#!/bin/env python3

import requests
import os
import shutil
import tarfile
from zipfile import ZipFile

toolsPath = os.path.abspath('_krita-tools')

baseToolsUrl = 'https://files.kde.org/krita/build/dependencies/'

cmakeArchive = 'cmake-3.29.3-macos-universal.tar.gz'
ninjaArchive = 'ninja-mac-1.12.1.zip'
ccacheArchive = 'ccache-4.9.1-darwin.tar.gz'

downloadsDir = os.environ.pop('EXTERNALS_DOWNLOAD_DIR')

def download_url(url, filePath, useCompression = False):
    headers = {'Accept-Encoding': 'gzip, deflate'} if useCompression else {}
    
    with requests.get(url, stream=True, headers=headers) as r:
        r.raise_for_status()
        with open(filePath, 'wb') as f:
            for chunk in r.iter_content(chunk_size=8192): 
                # If you have chunk encoded response uncomment if
                # and set chunk_size parameter to None.
                #if chunk: 
                f.write(chunk)
            f.flush()

def strip_extension(fn: str, extensions=[".tar.bz2", ".tar.gz"]):
    for ext in extensions:
        if fn.endswith(ext):
            return fn[: -len(ext)]
    raise ValueError(f"Unexpected extension for filename: {fn}")

for archive in [cmakeArchive, ninjaArchive, ccacheArchive]:
    targetFilePath = os.path.join(downloadsDir, archive)
    if not os.path.exists(targetFilePath):
        print (f'## Downloading {archive}...')
        download_url(baseToolsUrl + '/' + archive, targetFilePath)
    else:
        print (f'## Using cached download for {archive}...')

if os.path.isdir(toolsPath):
    print('## Removing old _krita-tools: {}'.format(toolsPath))
    shutil.rmtree(toolsPath)

os.makedirs(toolsPath)

print(f'## Extracting {cmakeArchive}')
with tarfile.open( name=os.path.join(downloadsDir, cmakeArchive), mode='r' ) as archive:
    archive.extractall( path=toolsPath )

print(f'## Extracting {ccacheArchive}')
with tarfile.open( name=os.path.join(downloadsDir, ccacheArchive), mode='r' ) as archive:
    archive.extractall( path=toolsPath )

print(f'## Extracting {ninjaArchive}')
with ZipFile( os.path.join(downloadsDir, ninjaArchive), mode='r' ) as archive:
    archive.extractall( path=os.path.join(toolsPath, 'ninja' ))
os.chmod(os.path.join(toolsPath, 'ninja', 'ninja'), 0o744)

with open(os.path.join(toolsPath, 'activate'), mode='w') as envFile:
    path = os.path.join(toolsPath, 'ninja')
    envFile.write(f'export PATH={path}:$PATH\n')
    path = os.path.join(toolsPath, strip_extension(cmakeArchive), 'CMake.app', 'Contents', 'bin')
    envFile.write(f'export PATH={path}:$PATH\n')
    path = os.path.join(toolsPath, strip_extension(ccacheArchive))
    envFile.write(f'export PATH={path}:$PATH\n')

