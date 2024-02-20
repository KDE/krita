#!/bin/python3

import requests

import shutil
import os
from sys import platform

projectArtifact = ""
depsUrl = ""

if platform == "linux":
    import tarfile
    projectArtifact = "krita-appimage-deps.tar"
    depsUrl = "https://files.kde.org/krita/dependencies/krita-appimage-deps.tar"
elif platform == "darwin":
    raise RuntimeError("MacOS support is not yet implemented")
elif platform == "win32":
    import zipfile
    projectArtifact = "krita-windows-deps.tar" # yes, even though the extension in .tar, is it still .zip...
    depsUrl = "https://files.kde.org/krita/dependencies/krita-windows-deps.tar"
    

localCachePath = os.environ.pop('KDECI_CACHE_PATH')

def download_url_with_compression(url, filePath):
    headers = {'Accept-Encoding': 'gzip, deflate'}
    
    with requests.get(depsUrl, stream=True, headers=headers) as r:
        r.raise_for_status()
        with open(filePath, 'wb') as f:
            for chunk in r.iter_content(chunk_size=8192): 
                # If you have chunk encoded response uncomment if
                # and set chunk_size parameter to None.
                #if chunk: 
                f.write(chunk)
            f.flush()

kritaCacheDir = os.path.join(localCachePath, 'krita-deps')
if not os.path.isdir(kritaCacheDir):
    os.makedirs(kritaCacheDir)

filePath = os.path.join(kritaCacheDir, projectArtifact)
urlFilePath = os.path.join(kritaCacheDir, '{}.url'.format(projectArtifact))

shouldDownloadNewFile = True

print("Cached file: {} (exists: {})".format(filePath, os.path.isfile(filePath)))
print("Cached URL file: {} (exists: {})".format(urlFilePath, os.path.isfile(urlFilePath)))

if os.path.isfile(filePath) and os.path.isfile(urlFilePath):
    existingFileUrl = ''
    with open(urlFilePath, 'r') as f:
        existingFileUrl = f.read()

    print("Existing file: {}".format(urlFilePath))
    print("Existing file URL: {}".format(existingFileUrl))

    shouldDownloadNewFile = existingFileUrl != depsUrl

print ("Should download the deps: {}".format(shouldDownloadNewFile))

if shouldDownloadNewFile:
    print ("Downloading deps: {}".format(depsUrl))

    if os.path.isfile(filePath):
        os.remove(filePath)
    if os.path.isfile(urlFilePath):
        os.remove(urlFilePath)

    download_url_with_compression(depsUrl, filePath)
    with open(urlFilePath, 'w') as f:
        f.write(depsUrl)
        f.flush()

print ("Extracting deps...")

file = None

if platform == "linux":
    file = tarfile.open(filePath, mode='r')
elif platform == "darwin":
    raise RuntimeError("MacOS support is not yet implemented")
elif platform == "win32":
    file = zipfile.ZipFile(filePath, mode='r')
    
file.extractall(path='_tmp')
file.close()

if os.path.isdir('_install'):
    shutil.rmtree('_install')

if platform == "linux":
    shutil.move('_tmp/deps/usr', '_install')
elif platform == "win32":
    shutil.move('_tmp\\deps-install', '_install')

    shutil.rmtree('_tmp')
