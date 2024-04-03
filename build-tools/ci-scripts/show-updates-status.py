#!/bin/python3

import requests

def check_url(url):
    try:
        with requests.get(url, stream=True) as r:
            return r.status_code == 200
    except:
        return False

def download_until_double_newline(url):
    downloadedFile = bytes()

    with requests.get(url, stream=True) as r:
        r.raise_for_status()
        for chunk in r.iter_content(chunk_size=16):
            if downloadedFile and chunk[0] == '\n' and downloadedFile[-1] == '\n':
                break
            
            index = chunk.find(b'\n\n')

            if index >= 0:
                downloadedFile = downloadedFile + chunk[:index]
                break
            else:
                downloadedFile = downloadedFile + chunk

    return downloadedFile


def getZSyncValues(url):
    file = download_until_double_newline(url)
    return dict(tuple(line.split(':', 1)) for line in file.decode().splitlines())

def printZSyncStat(title, url):
    zsyncExists = check_url(url)
    appimageExists = False
    values = {}

    if zsyncExists:
        values = getZSyncValues(url)
        appimageExists = check_url(values['URL'])
    
    print('== {}{} =='.format(title, ' FAILED' if not appimageExists or not zsyncExists else ''))
    print('ZSync URL: {}'.format(url))
    print('ZSync exists: {}'.format(zsyncExists))
    print('AppImage exists: {}'.format(appimageExists))

    if zsyncExists:
        for key in ['MTime', 'Filename', 'URL', 'SHA-1']:
            print('    {}: {}'.format(key, values[key]))

print('')
printZSyncStat("Channel: Stable", 'https://download.kde.org/stable/krita/updates/Krita-Stable-x86_64.appimage.zsync')
print('')
printZSyncStat("Channel: Beta (unstable)", 'https://download.kde.org/unstable/krita/updates/Krita-Beta-x86_64.appimage.zsync')
print('')
printZSyncStat("Channel: Plus", 'https://cdn.kde.org/ci-builds/graphics/krita/krita/5.2/linux/Krita-Plus-x86_64.appimage.zsync')
print('')
printZSyncStat("Channel: Next", 'https://cdn.kde.org/ci-builds/graphics/krita/master/linux/Krita-Next-x86_64.appimage.zsync')
print('')
