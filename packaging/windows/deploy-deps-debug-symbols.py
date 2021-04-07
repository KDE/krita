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
path_options.add_argument("--deps-install-dir", action='store',
                          help="Specify deps install dir")
path_options.add_argument("--deps-build-dir", action='store',
                          help="Specify deps build dir")
args = parser.parse_args()

DEPS_BUILD_DIR = None

if args.deps_build_dir is not None:
    DEPS_BUILD_DIR = args.deps_build_dir
if DEPS_BUILD_DIR is None:
    DEPS_BUILD_DIR = f"{os.getcwd()}\\b_deps_msvc"
    print(f"Using default deps build dir: {DEPS_BUILD_DIR}")
    if not args.no_interactive:
        status = choice()
        if not status:
            DEPS_BUILD_DIR = prompt_for_dir(
                "Provide path of deps build dir")
    if DEPS_BUILD_DIR is None:
        warnings.warn("ERROR: Deps build dir not set!")
        exit(102)
print(f"Deps build dir: {DEPS_BUILD_DIR}")

DEPS_INSTALL_DIR = None

if args.deps_install_dir is not None:
    DEPS_INSTALL_DIR = args.deps_install_dir
if DEPS_INSTALL_DIR is None:
    DEPS_INSTALL_DIR = f"{os.getcwd()}\\i_deps_msvc"
    print(f"Using default deps install dir: {DEPS_INSTALL_DIR}")
    if not args.no_interactive:
        status = choice()
        if not status:
            DEPS_INSTALL_DIR = prompt_for_dir(
                "Provide path of deps install dir")
    if DEPS_INSTALL_DIR is None:
        warnings.warn("ERROR: Deps install dir not set!")
        exit(102)
print(f"Deps install dir: {DEPS_INSTALL_DIR}")

# Simple checking
if not os.path.isdir(DEPS_INSTALL_DIR):
    warnings.warn("ERROR: Cannot find the selected install folder!")
    exit(1)
if not os.path.isdir(DEPS_BUILD_DIR):
    warnings.warn("ERROR: Cannot find the selected build folder!")
    exit(1)
# Amyspark: paths with spaces are automagically handled by Python!

if not args.no_interactive:
    status = choice()
    if not status:
        exit(255)
    print("")

# Glob all PDBs for Krita binaries

print("Listing all available PDB files...")

deps_pdb = glob.glob(
    f"{DEPS_BUILD_DIR}\\**\\*.pdb", recursive=True)

print("{} PDB files found in {}.".format(len(deps_pdb), DEPS_BUILD_DIR))

# Glob all binaries; we'll need to match pairs

print("Listing all available dependencies binaries...")

bin_exe = glob.glob(
    f"{DEPS_INSTALL_DIR}\\bin\\**\\*.exe", recursive=True)

bin_dll = glob.glob(
    f"{DEPS_INSTALL_DIR}\\bin\\**\\*.dll", recursive=True)

lib_dll = glob.glob(
    f"{DEPS_INSTALL_DIR}\\lib\\**\\*.dll", recursive=True)

lib_pyd = glob.glob(
    f"{DEPS_INSTALL_DIR}\\lib\\**\\*.pyd", recursive=True)

print(f"{len(bin_exe) + len(bin_dll) + len(lib_dll) + len(lib_pyd)} executable code files found.")

deps_binaries = {os.path.splitext(os.path.basename(f))[0]: os.path.dirname(
    f) for f in itertools.chain(bin_exe, bin_dll, lib_dll, lib_pyd)}

print("Matching up...")

for src in deps_pdb:
    key = os.path.splitext(os.path.basename(src))[0]
    dst = deps_binaries.get(key)
    if not dst:
        continue
    print(f"{src} => {dst}")
    shutil.copy(src, dst)
