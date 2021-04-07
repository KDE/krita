#!/usr/bin/env python

# SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
# SPDX-License-Identifier: GPL-2.0-or-later

# This Python script tries to retrieve all missing PDBs for
# existing EXEs or DLLs from their build folders

import argparse
import glob
import itertools
import os
import shutil
import warnings

# Subroutines


def choice(prompt="Is this ok?"):
    c = input(f"{prompt} [y/n] ")
    if c == "y":
        return True
    else:
        return False


def prompt_for_dir(prompt):
    user_input = input(f"{prompt}: ")
    if not len(user_input):
        return None
    result = os.path.exists(user_input)
    if result is None:
        print("Input does not point to valid dir!")
        return prompt_for_dir(prompt)
    else:
        return os.path.realpath(user_input)


# command-line args parsing
parser = argparse.ArgumentParser()

basic_options = parser.add_argument_group("Basic options")
basic_options.add_argument("--no-interactive", action='store_true',
                           help="Run without interactive prompts. When not specified, the script will prompt for some of the parameters.")
path_options = parser.add_argument_group("Path options")
path_options.add_argument("--krita-install-dir", action='store',
                          help="Specify Krita install dir")
path_options.add_argument(
    "--krita-build-dir", action="store", help="Specify Krita build dir")
args = parser.parse_args()

KRITA_BUILD_DIR = None

if args.krita_build_dir is not None:
    KRITA_BUILD_DIR = args.krita_build_dir
if KRITA_BUILD_DIR is None:
    KRITA_BUILD_DIR = f"{os.getcwd()}\\b_msvc"
    print(f"Using default Krita build dir: {KRITA_BUILD_DIR}")
    if not args.no_interactive:
        status = choice()
        if not status:
            KRITA_BUILD_DIR = prompt_for_dir(
                "Provide path of Krita build dir")
    if KRITA_BUILD_DIR is None:
        warnings.warn("ERROR: Krita build dir not set!")
        exit(102)
print(f"Krita build dir: {KRITA_BUILD_DIR}")

KRITA_INSTALL_DIR = None

if args.krita_install_dir is not None:
    KRITA_INSTALL_DIR = args.krita_install_dir
if KRITA_INSTALL_DIR is None:
    KRITA_INSTALL_DIR = f"{os.getcwd()}\\i_msvc"
    print(f"Using default Krita install dir: {KRITA_INSTALL_DIR}")
    if not args.no_interactive:
        status = choice()
        if not status:
            KRITA_INSTALL_DIR = prompt_for_dir(
                "Provide path of Krita install dir")
    if KRITA_INSTALL_DIR is None:
        warnings.warn("ERROR: Krita install dir not set!")
        exit(102)
print(f"Krita install dir: {KRITA_INSTALL_DIR}")

# Simple checking
if not os.path.isdir(KRITA_INSTALL_DIR):
    warnings.warn("ERROR: Cannot find the selected install folder!")
    exit(1)
if not os.path.isdir(KRITA_BUILD_DIR):
    warnings.warn("ERROR: Cannot find the selected build folder!")
    exit(1)
# Amyspark: paths with spaces are automagically handled by Python!

if not args.no_interactive:
    status = choice()
    if not status:
        exit(255)
    print("")

# Glob all PDBs for Krita binaries

# try and figure out what configuration was used

configurations = ["MinSizeRel", "RelWithDebInfo", "Release", "Debug"]

configuration = None

for c in configurations:
    if os.path.isfile(f"{KRITA_BUILD_DIR}\\bin\\{c}\\krita.pdb"):
        configuration = c
        break

if configuration is None:
    warnings.warn("ERROR: No compatible configuration was built!")
    exit(102)


print("Listing all available PDB files...")

pdb = glob.glob(
    f"{KRITA_BUILD_DIR}\\bin\\{configuration}\\*.pdb", recursive=False)

print("{} PDB files found in {}.".format(len(pdb), KRITA_BUILD_DIR))

# Glob all binaries; we'll need to match pairs

print("Listing all available Krita binaries...")

bin_exe = glob.glob(
    f"{KRITA_INSTALL_DIR}\\bin\\*.exe", recursive=False)

bin_dll = glob.glob(
    f"{KRITA_INSTALL_DIR}\\bin\\**\\*.dll", recursive=True)

lib_dll = glob.glob(
    f"{KRITA_INSTALL_DIR}\\lib\\**\\*.dll", recursive=True)

print(f"{len(bin_exe) + len(bin_dll) + len(lib_dll)} executable code files found.")

krita_binaries = {os.path.splitext(os.path.basename(f))[0]: os.path.dirname(
    f) for f in itertools.chain(bin_exe, bin_dll, lib_dll)}

print("Matching up...")

for src in pdb:
    key = os.path.splitext(os.path.basename(src))[0]
    if key == "PyKrita.krita":
       # This one is manually installed, continue.
       continue
    dst = krita_binaries[key]
    if not dst:
        continue
    if os.path.isfile(f"{dst}\\{os.path.basename(src)}"):
        print(f"{src} => {dst} ")
        continue
    else:
        print(f"{src} => {dst}")
        shutil.copy(src, dst)
