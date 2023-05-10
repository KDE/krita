#!/bin/python3

import requests
import tarfile
import shutil
import os

localCachePath = os.environ.pop('KDECI_CACHE_PATH')

def download_url_with_compression(url, filePath):
    headers = {'Accept-Encoding': 'gzip, deflate'}
    res = requests.get(depsUrl, headers=headers)

    if not res:
        res.raise_for_status()
	
    with open(filePath, 'wb') as f:
        f.write(res.content)
        f.flush()

print ("Fetching job metadata...")

res = requests.get("https://binary-factory.kde.org/job/Krita_Nightly_Appimage_Dependency_Build/api/json/")

if not res:
    print("Couldn't fetch the API object: {}".format(res.reason))
    res.raise_for_status

jobUrl = res.json()['lastSuccessfulBuild']['url']

print ("Last successful job URL: {}".format(jobUrl))

if jobUrl is None:
    raise FileNotFoundError()

depsUrl = '{}artifact/krita-appimage-deps.tar'.format(jobUrl)

kritaCacheDir = os.path.join(localCachePath, 'krita-deps')
if not os.path.isdir(kritaCacheDir):
    os.makedirs(kritaCacheDir)

filePath = os.path.join(kritaCacheDir, 'krita-appimage-deps.tar')
urlFilePath = os.path.join(kritaCacheDir, 'krita-appimage-deps.tar.url')

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

file = tarfile.open(filePath, mode='r')
file.extractall(path='_tmp')
file.close()

if os.path.isdir('_install'):
    shutil.rmtree('_install')
shutil.move('_tmp/deps/usr', '_install')
shutil.rmtree('_tmp')
