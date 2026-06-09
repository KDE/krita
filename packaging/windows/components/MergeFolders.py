#!/bin/env python3

#
# WARNING: the file is copied from krita-ci-utilities repo, please keep it in sync
# from https://invent.kde.org/packaging/krita-ci-utilities/-/blob/master/components/MergeFolders.py?ref_type=heads
#

import shutil
import glob
import os
import fnmatch
import re

def merge_folders(srcDir, dstDir, move_files = False, skip_paths = []):

    copy_function = shutil.move \
        if move_files \
        else lambda s,d: shutil.copy2(s, d, follow_symlinks=False)

    if not os.path.exists(dstDir):
        os.makedirs(dstDir)

    normedSkipPath = [os.path.normpath(os.path.join(srcDir, path)) for path in skip_paths]
    # print("Merging paths: {} -> {}".format(srcDir, dstDir))
    # print("    ignored paths: {}".format(skip_paths))
    # print("    ignored paths (resolved): {}".format(normedSkipPath))

    for root, dirs, files in os.walk(srcDir):
        #print('{} {} {}'.format(root, dirs, files))
        
        for dir in list(dirs):
            srcPath = os.path.join(root, dir)
            if os.path.normpath(srcPath) in normedSkipPath:
                # print("INFO: skipping ignored directory: {}".format(srcPath))
                dirs.remove(dir)
                continue
            dstPath = os.path.join(dstDir, os.path.relpath(srcPath, srcDir))

            # print("src: {}".format(srcPath))
            # print("dst: {}".format(dstPath))

            if not os.path.exists(dstPath):
                # just copy normally if destination doesn't exist
                if os.path.islink(srcPath):
                    #print('# copy symlink {} -> {}'.format(srcPath, dstPath))
                    copy_function(srcPath, dstPath)
                else:
                    #print('# mkdirs {}'.format(dstPath))
                    os.makedirs(dstPath)
            elif not os.path.isdir(dstPath):
                # we cannot copy a folder into a file
                print("src path: {}".format(srcPath))
                print("dst path: {}".format(dstPath))
                raise("Couldn't override a file with a folder")
            elif os.path.islink(srcPath) and os.path.islink(dstPath):
                srcLink = os.readlink(srcPath)
                dstLink = os.readlink(dstPath)

                if srcLink == dstLink:
                    dirs.remove(dir)
                else:
                    print("src path: {}".format(srcPath))
                    print("dst path: {}".format(dstPath))
                    raise("Couldn't override a symlink with a different path")
            elif not os.path.islink(srcPath) and os.path.islink(dstPath):
                # copy the content of the physical folder into symlink
                pass
            elif os.path.islink(srcPath) and not os.path.islink(dstPath):
                # we cannot copy a symlink into a physical folder, error out!
                print("src path: {}".format(srcPath))
                print("dst path: {}".format(dstPath))
                raise("Couldn't override a physical folder with a symlink")
            else:
                # destination folder exists
                pass
        
        
        for file in files:
            srcPath = os.path.join(root, file)
            if os.path.normpath(srcPath) in normedSkipPath:
                # print("INFO: skipping ignored file: {}".format(srcPath))
                continue
            dstPath = os.path.join(dstDir, os.path.relpath(srcPath, srcDir))

            if not os.path.exists(dstPath):
                # just copy normally if destination doesn't exist
                #print('# copy file {} -> {}'.format(srcPath, dstPath))
                copy_function(srcPath, dstPath)
            
            elif not os.path.isfile(dstPath):
                print("src path: {}".format(srcPath))
                print("dst path: {}".format(dstPath))
                raise FileExistsError("Couldn't override not-a-file with a file")

            elif os.path.islink(srcPath) and os.path.islink(dstPath):
                srcLink = os.readlink(srcPath)
                dstLink = os.readlink(dstPath)

                if srcLink != dstLink:
                    print("ERROR: cannot overwrite a symlink with a symlink pointing to a different path")
                    print("src path: {}".format(srcPath))
                    print("dst path: {}".format(dstPath))
                    raise FileExistsError("Couldn't override a symlink with a different path")

            else:
                ignoredPatterns = [
                    r'.*/_vendor/.*\.py',
                    '.*/site-packages/setuptools/.*',
                    '/__pycache__/',
                    '.*site-packages/.*distutils.*',
                    '.*site-packages/pkg_resources.*'
                ]

                patternString = '|'.join(map(lambda x: f'({x})', ignoredPatterns))
                pattern = re.compile(patternString)

                if os.environ.get('KDECI_DEBUG_OVERWRITTEN_FILES', 'no').lower() in ['true', '1', 't', 'y', 'yes'] and \
                   not fnmatch.fnmatch(file, '*.pyc') and \
                    not pattern.match(dstPath):
                    print ('WARNING: overwriting a file: {} -> {}'.format(srcPath, dstPath))

                #print('# overwrite file {} -> {}'.format(srcPath, dstPath))
                copy_function(srcPath, dstPath)

                # print("src path: {}".format(srcPath))
                # print("dst path: {}".format(dstPath))
                # raise("Couldn't override a file")


# srcDir = './python-modules'
# dstDir = './python'

# merge_folders(srcDir, dstDir, move_files=True)