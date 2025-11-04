#!/bin/python3

import os
import argparse
import shutil
import glob

# Capture our command line parameters
parser = argparse.ArgumentParser(description='A script for moving away signing/notarization logs')
parser.add_argument('-p', '--prefix', type=str, required=True, help='A prefix that should be added to the log\'s file name')
parser.add_argument('-d', '--directory', type=str, required=False, help='A directory where to look for log files')
parser.add_argument('-o', '--destination', type=str, required=False, help='A directory where to put the files')
arguments = parser.parse_args()

if arguments.directory is None:
    arguments.directory = os.getcwd()
if arguments.destination is None:
    arguments.destination = os.getcwd()

if not os.path.exists(arguments.destination):
    os.makedirs(arguments.destination)

if os.path.abspath(arguments.directory) == os.path.abspath(arguments.destination):
    raise Exception("Source and destination directories are the same")

for file in glob.glob(os.path.join(arguments.directory, '*.log'), recursive=True):
    newFileName = f"{arguments.prefix}{os.path.basename(file)}"
    newFilePath = os.path.join(arguments.destination, newFileName)

    print(f"## Moving a log file: {file} -> {newFilePath}")
    shutil.move(file, newFilePath)
