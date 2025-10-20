#!/usr/bin/env python

# SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
# SPDX-License-Identifier: GPL-2.0-or-later

# This Python script is meant to prepare a Krita package folder to be zipped or
# to be a base for the installer.

import argparse
import glob
import itertools
import os
import re
import pathlib
import shutil
import subprocess
import sys
import warnings


# Subroutines

def choice(prompt="Is this ok?"):
    c = input(f"{prompt} [y/n] ")
    if c == "y":
        return True
    else:
        return False


def find_on_path(variable, executable):
    exeString = shutil.which(executable)
    if exeString:
        os.environ[variable] = exeString


def prompt_for_file(prompt):
    user_input = input(f"{prompt} ")
    if not len(user_input):
        return None
    result = os.path.isfile(user_input)
    if result is None:
        print("Input does not point to valid dir!")
        return prompt_for_file(prompt)
    else:
        return os.path.realpath(user_input)


def prompt_for_dir(prompt):
    user_input = input(f"{prompt} ")
    if not len(user_input):
        return None
    result = os.path.isdir(user_input)
    if result is None:
        print("Input does not point to valid dir!")
        return prompt_for_dir(prompt)
    else:
        return os.path.realpath(user_input)


print(f"Krita Windows packaging script")


# command-line args parsing
parser = argparse.ArgumentParser()

basic_options = parser.add_argument_group("Basic options")
basic_options.add_argument("--no-interactive", action='store_true',
                           help="Run without interactive prompts. When not specified, the script will prompt for some of the parameters.")
basic_options.add_argument(
    "--package-name", action='store', help="Specify the package name", required=True)

path_options = parser.add_argument_group("Path options")
path_options.add_argument("--src-dir", action='store',
                          help="Specify Krita source dir. If unspecified, this will be determined from the script location")
path_options.add_argument("--deps-install-dir", action='store',
                          help="Specify deps install dir")
path_options.add_argument("--krita-install-dir", action='store',
                          help="Specify Krita install dir")

special_options = parser.add_argument_group("Special options")
special_options.add_argument("--pre-zip-hook", action='store',
                             help="Specify a script to be called before packaging the zip archive, can be used to sign the binaries")

args = parser.parse_args()

# Check environment config

if os.environ.get("SEVENZIP_EXE") is None:
    find_on_path("SEVENZIP_EXE", "7z.exe")
if os.environ.get("SEVENZIP_EXE") is None:
    find_on_path("SEVENZIP_EXE", "7za.exe")
if os.environ.get("SEVENZIP_EXE") is None:
    os.environ["SEVENZIP_EXE"] = f"{os.environ['ProgramFiles']}\\7-Zip\\7z.exe"
    if not os.path.isfile(os.environ["SEVENZIP_EXE"]):
        os.environ["SEVENZIP_EXE"] = "{}\\7-Zip\\7z.exe".format(
            os.environ["ProgramFiles(x86)"])
    if not os.path.isfile(os.environ["SEVENZIP_EXE"]):
        warnings.warn("7-Zip not found!")
        sys.exit(102)
print(f"7-Zip: {os.environ['SEVENZIP_EXE']}")

if not os.environ.get("MINGW_BIN_DIR"):
    find_on_path("MINGW_BIN_DIR_MAKE_EXE", "mingw32-make.exe")
    if not os.environ.get('MINGW_BIN_DIR_MAKE_EXE'):
        if not args.no_interactive:
            os.environ['MINGW_BIN_DIR_MAKE_EXE'] = prompt_for_file("Provide path to mingw32-make.exe of mingw-w64")
        if not os.environ.get('MINGW_BIN_DIR_MAKE_EXE'):
            warnings.warn("ERROR: mingw-w64 not found!")
            sys.exit(102)
        os.environ['MINGW_BIN_DIR'] = os.path.dirname(os.environ['MINGW_BIN_DIR_MAKE_EXE'])
    else:
        os.environ['MINGW_BIN_DIR'] = os.path.dirname(os.environ['MINGW_BIN_DIR_MAKE_EXE'])
        print(f"Found mingw on PATH: {os.environ['MINGW_BIN_DIR_MAKE_EXE']}")
        if not args.no_interactive:
            status = choice("Is this correct?")
            if not status:
                os.environ['MINGW_BIN_DIR_MAKE_EXE'] = prompt_for_dir("Provide path to mingw32-make.exe of mingw-w64")
            if not os.environ['MINGW_BIN_DIR_MAKE_EXE']:
                warnings.warn("ERROR: mingw-w64 not found!")
                sys.exit(102)
        os.environ['MINGW_BIN_DIR'] = os.path.dirname(os.environ['MINGW_BIN_DIR_MAKE_EXE'])
print(f"mingw-w64: {os.environ['MINGW_BIN_DIR']}")
        
# Windows SDK is needed for windeployqt to get d3dcompiler_xx.dll
# If we don't define this variable, windeployqt will fetch the library
# from  %WINDIR%\system32, which is not supposed to be redistributable
if os.environ.get("WindowsSdkDir") is None and os.environ.get("ProgramFiles(x86)") is not None:
    os.environ["WindowsSdkDir"] = "{}\\Windows Kits\\10".format(
        os.environ["ProgramFiles(x86)"])

KRITA_SRC_DIR = None

if args.src_dir is not None:
    KRITA_SRC_DIR = args.src_dir

if KRITA_SRC_DIR is None:
    _temp = sys.argv[0]
    if os.path.dirname(_temp).endswith("\\packaging\\windows"):
        _base = pathlib.PurePath(_temp)
        if os.path.isfile(f"{_base.parents[2]}\\CMakeLists.txt"):
            if os.path.isfile(f"{_base.parents[2]}\\libs\\version\\kritaversion.h.cmake"):
                KRITA_SRC_DIR = os.path.realpath(_base.parents[2])
                print("Script is running inside Krita source dir")

if KRITA_SRC_DIR is None:
    if args.no_interactive:
        KRITA_SRC_DIR = prompt_for_dir("Provide path of Krita src dir")
    if KRITA_SRC_DIR is None:
        warnings.warn("ERROR: Krita src dir not found!")
        sys.exit(102)
print(f"Krita src: {KRITA_SRC_DIR}")

DEPS_INSTALL_DIR = None

if args.deps_install_dir is not None:
    DEPS_INSTALL_DIR = args.deps_install_dir
if DEPS_INSTALL_DIR is None:
    DEPS_INSTALL_DIR = f"{os.getcwd()}\\_install"
    print(f"Using default deps install dir: {DEPS_INSTALL_DIR}")
    if not args.no_interactive:
        status = choice()
        if not status:
            DEPS_INSTALL_DIR = prompt_for_dir(
                "Provide path of deps install dir")
    if DEPS_INSTALL_DIR is None:
        warnings.warn("ERROR: Deps install dir not set!")
        sys.exit(102)
print(f"Deps install dir: {DEPS_INSTALL_DIR}")

KRITA_INSTALL_DIR = None

if args.krita_install_dir is not None:
    KRITA_INSTALL_DIR = args.krita_install_dir
if KRITA_INSTALL_DIR is None:
    KRITA_INSTALL_DIR = f"{os.getcwd()}\\_install"
    print(f"Using default Krita install dir: {KRITA_INSTALL_DIR}")
    if not args.no_interactive:
        status = choice()
        if not status:
            KRITA_INSTALL_DIR = prompt_for_dir(
                "Provide path of Krita install dir")
    if KRITA_INSTALL_DIR is None:
        warnings.warn("ERROR: Krita install dir not set!")
        sys.exit(102)
print(f"Krita install dir: {KRITA_INSTALL_DIR}")

# Simple checking
if not os.path.isdir(DEPS_INSTALL_DIR):
    warnings.warn("ERROR: Cannot find the deps install folder!")
    sys.exit(1)
if not os.path.isdir(KRITA_INSTALL_DIR):
    warnings.warn("ERROR: Cannot find the krita install folder!")
    sys.exit(1)
# Amyspark: paths with spaces are automagically handled by Python!

pkg_name = args.package_name
print(f"Package name is {pkg_name}")

pkg_root = f"{os.getcwd()}\\{pkg_name}"
print(f"Packaging dir is {pkg_root}\n")
if os.path.isdir(pkg_root):
    warnings.warn(
        "ERROR: Packaging dir already exists! Please remove or rename it first.")
    sys.exit(1)
if os.path.isfile(f"{pkg_root}.zip"):
    warnings.warn(
        "ERROR: Packaging zip already exists! Please remove or rename it first.")
    sys.exit(1)
if os.path.isfile(f"{pkg_root}-dbg.zip"):
    warnings.warn(
        "ERROR: Packaging debug zip already exists! Please remove or rename it first.")
    sys.exit(1)

if not args.no_interactive:
    status = choice()
    if not status:
        sys.exit(255)
    print("")

# Initialize clean PATH
os.environ["PATH"] = fr"{os.environ['SystemRoot']}\system32;{os.environ['SystemRoot']};{os.environ['SystemRoot']}\System32\Wbem;{os.environ['SystemRoot']}\System32\WindowsPowerShell\v1.0" + "\\"
os.environ["PATH"] = fr"{os.environ['MINGW_BIN_DIR']};{DEPS_INSTALL_DIR}\bin;{os.environ['PATH']}"


print("\nTrying to guess compiler version...")
for arg in ("g++", "clang++"):
    commandToRun = f"{arg} --version"
    ret = subprocess.call(commandToRun, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, shell=True)
    if ret != 1:
        os.environ['CC'] = arg
if not os.environ.get('CC'):
    warnings.warn("ERROR: No working compiler.")
    sys.exit(1)
output = subprocess.check_output(f"{os.environ['CC']} --version", shell=True, text=True)
GCCversionLines = output.splitlines()
GCC_VERSION_LINE = ""
for versionLine in GCCversionLines:
    if re.search("g\+\+", versionLine):
        GCC_VERSION_LINE = versionLine
# The space in "clang " is essential to detect
# the version line and not the InstalledDir
if not GCC_VERSION_LINE:
    for versionLine in GCCversionLines:
        if re.search("clang ", versionLine):
            GCC_VERSION_LINE = versionLine
if not GCC_VERSION_LINE:
    warnings.warn("ERROR: Failed to get version of g++.")
    sys.exit(1)
print(f"-- {GCC_VERSION_LINE}")
IS_TDM = False
IS_MSYS = False
IS_CLANG = False
if re.search("tdm64", GCC_VERSION_LINE):
    print("Compiler looks like TDM64-GCC")
    IS_TDM = True
if re.search("MSYS", GCC_VERSION_LINE):
    print("Compiler looks like MSYS GCC")
    IS_MSYS = True
if re.search("clang", GCC_VERSION_LINE):
    print("Compiler looks like Clang")
    IS_CLANG = True
else:
    print("Compiler looks like plain old MinGW64")
IS_LLVM_MINGW = False
IS_MSYS_CLANG = False
if IS_CLANG:
    # Look for the multiarch target binary dirs. Unfortunately there is
    # no surefire way to detect a llvm-mingw toolchain.
    if os.path.isdir(f"{os.environ['MINGW_BIN_DIR']}\\..\\i686-w64-mingw32\\bin\\") and \
       os.path.isdir(f"{os.environ['MINGW_BIN_DIR']}\\..\\x86_64-w64-mingw32\\bin\\"):
            print("Toolchain looks like llvm-mingw")
            IS_LLVM_MINGW = True
    else:
        print("Toolchain does not look like llvm-mingw, assuming MSYS Clang")
        IS_MSYS_CLANG = True

print("\nTrying to guess target architecture...")
OBJDUMP = False
for arg in ("objdump", "llvm-objdump"):
    commandToRun = f"{arg} --version"
    ret = subprocess.call(commandToRun, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, shell=True)
    if ret != 1:
        OBJDUMP = arg
if not OBJDUMP:
    warnings.warn("ERROR: objdump is not working.")
    sys.exit(1)
output = subprocess.check_output(fr"{OBJDUMP} -f {KRITA_INSTALL_DIR}\bin\krita.exe", shell=True, text=True)
targetArchLines = output.splitlines()

TARGET_ARCH_LINE = ""
for archLine in targetArchLines:
    if re.search("i386", archLine):
        TARGET_ARCH_LINE = archLine
    if not TARGET_ARCH_LINE:
        print("Possible LLVM objdump, trying to detect architecture...")
        if re.search("coff", archLine):
            TARGET_ARCH_LINE = archLine
print(f"-- {TARGET_ARCH_LINE}")
IS_x64 = False
if not re.search("x86-64", TARGET_ARCH_LINE):
    print("Target looks like x86")
else:
    print("Target looks like x86_64")
    IS_x64 = True

print("\nTesting for objcopy...")
commandToRun = "objcopy --version"
try:
    subprocess.check_call(commandToRun, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, shell=True)
except subprocess.CalledProcessError:
    warnings.warn("ERROR: objcopy is not working.")
    sys.exit(1)

print("\nTesting for strip...")
commandToRun = "strip --version"
try:
    subprocess.check_call(commandToRun, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, shell=True)
except subprocess.CalledProcessError:
    warnings.warn("ERROR: strip is not working.")
    sys.exit(1)

print("\nTrying to guess Qt version...", end = "")
useQt6Build = False
if os.path.exists(f"{DEPS_INSTALL_DIR}\\bin\\Qt6Core.dll"):
    print(" Found Qt6")
    useQt6Build = True
else:
    print(" Found Qt5")

print("\nCreating base directories...")

try:
    os.makedirs(f"{pkg_root}", exist_ok=True)
    os.makedirs(f"{pkg_root}\\bin", exist_ok=True)
    os.makedirs(f"{pkg_root}\\etc", exist_ok=True)
    os.makedirs(f"{pkg_root}\\lib", exist_ok=True)
    os.makedirs(f"{pkg_root}\\share", exist_ok=True)
except:
    warnings.warn("ERROR: Cannot create packaging dir tree!")
    sys.exit(1)

print("\nCopying GCC libraries...")
os.environ['STDLIBS_DIR'] = os.environ['MINGW_BIN_DIR']
if IS_TDM:
    if not IS_x64:
		# TDM-GCC x86
        os.environ['STDLIBS'] = "libgomp-1"
    else:
		# TDM-GCC x64
        os.environ['STDLIBS'] = "libgomp_64-1"
elif IS_MSYS:
    if not IS_x64:
		# msys-mingw-w64 x86
        os.environ['STDLIBS'] = "libgcc_s_dw2-1 libgomp-1 libstdc++-6 libwinpthread-1 libatomic-1 libiconv-2 zlib1 libexpat-1 libintl-8 libssl-1_1 libcrypto-1_1"
    else:
		# msys-mingw-w64 x64
        os.environ['STDLIBS'] = "libgcc_s_seh-1 libgomp-1 libstdc++-6 libwinpthread-1 libatomic-1 libiconv-2 zlib1 libexpat-1 libintl-8 libssl-1_1-x64 libcrypto-1_1-x64"
elif IS_MSYS_CLANG:
    if not IS_x64:
		# msys-mingw-w64-clang64 x86
        os.environ['STDLIBS'] = "libomp libssp-0 libunwind libc++ libwinpthread-1 libiconv-2 zlib1 libexpat-1 libintl-8 libssl-1_1 libcrypto-1_1"
    else:
		# msys-mingw-w64-clang64 x64
        os.environ['STDLIBS'] = "libomp libssp-0 libunwind libc++ libwinpthread-1 libiconv-2 zlib1 libexpat-1 libintl-8 libssl-1_1-x64 libcrypto-1_1-x64"
elif IS_LLVM_MINGW:
    # llvm-mingw
    os.environ['STDLIBS'] = "libc++ libomp libssp-0 libunwind libwinpthread-1"
    # The toolchain does not include all of the DLLs in the compiler bin dir,
    # so we need to copy them from the cross target bin dir.
    if not IS_x64:
        asanLibName ='libclang_rt.asan_dynamic-i386'
        os.environ['STDLIBS_DIR'] = fr"{os.environ['MINGW_BIN_DIR']}\..\i686-w64-mingw32\bin"
    else:
        asanLibName = 'libclang_rt.asan_dynamic-x86_64'
        os.environ['STDLIBS_DIR'] = fr"{os.environ['MINGW_BIN_DIR']}\..\x86_64-w64-mingw32\bin"

    output = subprocess.check_output(fr"{OBJDUMP} -p {KRITA_INSTALL_DIR}\bin\krita.exe", shell=True, text=True)

    if re.search(asanLibName, output):
        print('The package contains ASAN, packaging the ASAN runtime as well!')
        os.environ['STDLIBS'] = f'{os.environ["STDLIBS"]} {asanLibName}'

        symbolizerBinaries = ['libLLVM-18.dll', 'llvm-symbolizer.exe']
        for file in symbolizerBinaries:
            filePath = os.path.join(os.environ['MINGW_BIN_DIR'], file)

            print(f'Copying symbolizer lib: {filePath}')
            shutil.copy(filePath, fr"{pkg_root}\bin")
    else:
        print('No ASAN was found in the Krita binary, skipping its packaging...')

else:
    if not IS_x64:
		# mingw-w64 x86
        os.environ['STDLIBS'] = "libgcc_s_dw2-1 libgomp-1 libstdc++-6 libwinpthread-1"
    else:
		# mingw-w64 x64
        os.environ['STDLIBS'] = "libgcc_s_seh-1 libgomp-1 libstdc++-6 libwinpthread-1"

for lib in os.environ['STDLIBS'].split(" "):
    libPath = fr"{os.environ['STDLIBS_DIR']}\{lib}.dll"
    # libssp-0 is not present in clang18
    if lib != 'libssp-0' or os.path.isfile(libPath):
        shutil.copy(libPath, fr"{pkg_root}\bin")

print("\nCopying files...")
# krita.exe
shutil.copy(f"{KRITA_INSTALL_DIR}\\bin\\krita.exe", f"{pkg_root}\\bin\\")
shutil.copy(f"{KRITA_INSTALL_DIR}\\bin\\krita.com", f"{pkg_root}\\bin\\")
if os.path.isfile(f"{KRITA_INSTALL_DIR}\\bin\\krita.pdb"):
    shutil.copy(f"{KRITA_INSTALL_DIR}\\bin\\krita.pdb", f"{pkg_root}\\bin\\")
# kritarunner.exe
if os.path.isfile(f"{KRITA_INSTALL_DIR}\\bin\\kritarunner.exe"):
    shutil.copy(f"{KRITA_INSTALL_DIR}\\bin\\kritarunner.exe", f"{pkg_root}\\bin\\")
    if os.path.isfile(f"{KRITA_INSTALL_DIR}\\bin\\kritarunner.pdb"):
        shutil.copy(f"{KRITA_INSTALL_DIR}\\bin\\kritarunner.pdb",
                    f"{pkg_root}\\bin\\")
    shutil.copy(f"{KRITA_INSTALL_DIR}\\bin\\kritarunner.com",
                f"{pkg_root}\\bin\\")
if os.path.isfile(f"{KRITA_INSTALL_DIR}\\bin\\FreehandStrokeBenchmark.exe"):
    shutil.copy(f"{KRITA_INSTALL_DIR}\\bin\\FreehandStrokeBenchmark.exe", f"{pkg_root}\\bin\\")
    subprocess.run(["xcopy", "/S", "/Y", "/I",
                   f"{DEPS_INSTALL_DIR}\\bin\\data\\", f"{pkg_root}\\bin\\data\\"])

# qt.conf -- to specify the location to Qt translations
shutil.copy(fr"{KRITA_SRC_DIR}\packaging\windows\qt.conf", fr"{pkg_root}\bin")
# DLLs from bin/
print("INFO: Copying all DLLs except Qt5/Qt6 * from bin/")
files = glob.glob(f"{KRITA_INSTALL_DIR}\\bin\\*.dll")
pdbs = glob.glob(f"{KRITA_INSTALL_DIR}\\bin\\*.pdb")
for f in itertools.chain(files, pdbs):
    if not os.path.basename(f).startswith("Qt5") and not os.path.basename(f).startswith("Qt6"):
        shutil.copy(f, f"{pkg_root}\\bin")
files = glob.glob(f"{DEPS_INSTALL_DIR}\\bin\\*.dll")
for f in files:
    pdb = f"{os.path.dirname(f)}\\{os.path.splitext(os.path.basename(f))[0]}.pdb"
    if not os.path.basename(f).startswith("Qt5") and not os.path.basename(f).startswith("Qt6"):
        shutil.copy(f, f"{pkg_root}\\bin")
        if os.path.isfile(pdb):
            shutil.copy(pdb, f"{pkg_root}\\bin")
# symsrv.yes for Dr. Mingw
shutil.copy(f"{DEPS_INSTALL_DIR}\\bin\\symsrv.yes", f"{pkg_root}\\bin")
# DLLs from lib/
print("INFO: Copying all DLLs from lib/ (deps)")
files = glob.glob(fr"{DEPS_INSTALL_DIR}\lib\*.dll")
for f in files:
    shutil.copy(f, fr"{pkg_root}\bin")
for f in files:
    shutil.copy(f, fr"{pkg_root}\bin")
# KF5/KF6 plugins may be placed at different locations depending on how Qt is built
subprocess.run(["xcopy", "/S", "/Y", "/I",
               f"{DEPS_INSTALL_DIR}\\lib\\plugins\\imageformats\\", f"{pkg_root}\\bin\\imageformats\\"])
subprocess.run(["xcopy", "/S", "/Y", "/I", "{}\\plugins\\imageformats\\".format(
    DEPS_INSTALL_DIR), f"{pkg_root}\\bin\\imageformats\\"])

if useQt6Build:
    subprocess.run(["xcopy", "/S", "/Y", "/I", "{}\\plugins\\kf6\\".format(
        DEPS_INSTALL_DIR), f"{pkg_root}\\bin\\kf6\\"])
else:
    subprocess.run(["xcopy", "/S", "/Y", "/I", "{}\\plugins\\kf5\\".format(
        DEPS_INSTALL_DIR), f"{pkg_root}\\bin\\kf5\\"])

# Copy the sql drivers explicitly
subprocess.run(["xcopy", "/S", "/Y", "/I", "{}\\plugins\\sqldrivers\\".format(
    DEPS_INSTALL_DIR), f"{pkg_root}\\bin\\sqldrivers"], check=True)

# Qt Translations
# it seems that windeployqt does these, but only * some * of these???
os.makedirs(f"{pkg_root}\\bin\\translations", exist_ok=True)
files = glob.glob(f"{DEPS_INSTALL_DIR}\\translations\\qt_*.qm")
for f in files:
    # Exclude qt_help_*.qm
    if not os.path.basename(f).startswith("qt_help"):
        shutil.copy(f, f"{pkg_root}\\bin\\translations")

# Krita plugins
subprocess.run(["xcopy", "/Y", "{}\\lib\\kritaplugins\\*.dll".format(
    KRITA_INSTALL_DIR), f"{pkg_root}\\lib\\kritaplugins\\"], check=True)
subprocess.run(["xcopy", "/Y", "{}\\lib\\kritaplugins\\*.pdb".format(
    KRITA_INSTALL_DIR), f"{pkg_root}\\lib\\kritaplugins\\"], check=True)
if os.path.isdir(f"{DEPS_INSTALL_DIR}\\lib\\krita-python-libs"):
    subprocess.run(["xcopy", "/S", "/Y", "/I", "{}\\lib\\krita-python-libs".format(DEPS_INSTALL_DIR), f"{pkg_root}\\lib\\krita-python-libs"], check=True)
if os.path.isdir(f"{KRITA_INSTALL_DIR}\\lib\\krita-python-libs"):
    subprocess.run(["xcopy", "/S", "/Y", "/I", "{}\\lib\\krita-python-libs".format(
        KRITA_INSTALL_DIR), f"{pkg_root}\\lib\\krita-python-libs"], check=True)
if os.path.isdir(f"{DEPS_INSTALL_DIR}\\lib\\site-packages"):
    subprocess.run(["xcopy", "/S", "/Y", "/I", "{}\\lib\\site-packages".format(
        DEPS_INSTALL_DIR), f"{pkg_root}\\lib\\site-packages"], check=True)

# MLT plugins and their data
subprocess.run(["xcopy", "/S", "/Y", "/I", "{}\\lib\\mlt".format(
    DEPS_INSTALL_DIR), f"{pkg_root}\\lib\\mlt"], check=True)
subprocess.run(["xcopy", "/S", "/Y", "/I", "{}\\share\\mlt".format(
    DEPS_INSTALL_DIR), f"{pkg_root}\\share\\mlt"], check=True)

# Fontconfig
subprocess.run(["xcopy", "/S", "/Y", "/I", "{}\\etc\\fonts".format(
    DEPS_INSTALL_DIR), f"{pkg_root}\\etc\\fonts"], check=True)

# Share
subprocess.run(["xcopy", "/S", "/Y", "/I", "{}\\share\\color".format(
    KRITA_INSTALL_DIR), f"{pkg_root}\\share\\color"], check=True)
subprocess.run(["xcopy", "/S", "/Y", "/I", "{}\\share\\color-schemes".format(
    KRITA_INSTALL_DIR), f"{pkg_root}\\share\\color-schemes"], check=True)
subprocess.run(["xcopy", "/S", "/Y", "/I", "{}\\share\\icons".format(
    KRITA_INSTALL_DIR), f"{pkg_root}\\share\\icons"], check=True)
subprocess.run(["xcopy", "/S", "/Y", "/I", "{}\\share\\krita".format(
    KRITA_INSTALL_DIR), f"{pkg_root}\\share\\krita"])
subprocess.run(["xcopy", "/S", "/Y", "/I", "{}\\share\\kritaplugins".format(
    KRITA_INSTALL_DIR), f"{pkg_root}\\share\\kritaplugins"], check=True)

if useQt6Build:
    subprocess.run(["xcopy", "/S", "/Y", "/I", "{}\\share\\kf6".format(
        DEPS_INSTALL_DIR), f"{pkg_root}\\share\\kf6"], check=True)
else:
    subprocess.run(["xcopy", "/S", "/Y", "/I", "{}\\share\\kf5".format(
        DEPS_INSTALL_DIR), f"{pkg_root}\\share\\kf5"], check=True)

subprocess.run(["xcopy", "/S", "/Y", "/I", "{}\\share\\mime".format(
    DEPS_INSTALL_DIR), f"{pkg_root}\\share\\mime"], check=True)
# Python libs are copied by share\krita above
# Copy locale to bin
subprocess.run(["xcopy", "/S", "/Y", "/I", "{}\\share\\locale".format(
    KRITA_INSTALL_DIR), f"{pkg_root}\\bin\\locale"])
subprocess.run(["xcopy", "/S", "/Y", "/I", "{}\\share\\locale".format(
    DEPS_INSTALL_DIR), f"{pkg_root}\\bin\\locale"], check=True)

# Copy shortcut link from source (can't create it dynamically)
shutil.copy(f"{KRITA_SRC_DIR}\\packaging\\windows\\krita.lnk", pkg_root)
shutil.copy(
    f"{KRITA_SRC_DIR}\\packaging\\windows\\krita-minimal.lnk", pkg_root)
shutil.copy(
    f"{KRITA_SRC_DIR}\\packaging\\windows\\krita-animation.lnk", pkg_root)

# QML deployment:
#
# When deploying QML modules we should pass windeployqt all the folders
# **in the source tree** where our resource-embedded .qml files are situated.
# Every such folder should be declared with --qmldir option. These files will
# **not** be deployed (because they are expected to be stored as binary resources),
# but all their dependencies will be deployed to the package folder.
#
# Multiple QML dependencies search paths can be provided by --qmlimport switch,
# we don't pass it explicitly and let it be deduced by windeployqt using qtpath
# executable.

# Here we should list all the folders/plugins in Krita that have
# .qml files inside. Theoretically, we can just pass the entire Krita's
# source tree, but I'm not sure it is a good idea.
QMLDIR_ARGS = ["--qmldir", fr"{KRITA_SRC_DIR}\plugins\dockers\textproperties"]

# A safeguard for the case when KDE_INSTALL_USE_QT_SYS_PATHS is not properly 
# activated on Windows and the QML modules are installed into a default KDE's
# location instead of the one returned by qtpath. If you see this error you 
# should either recreate your build tree, or pass -DKDE_INSTALL_USE_QT_SYS_PATHS=ON
# to the build or check if you have non-standard qt.conf in your installation 
# root.

if os.path.isdir(fr"{KRITA_INSTALL_DIR}\lib\qml"):
    print("ERROR: some of Krita's QML modules were installed in an incorrect location")
    print(fr"    actual path: {KRITA_INSTALL_DIR}\lib\qml")
    print(fr"    expected path: {KRITA_INSTALL_DIR}\qml")
    exit(103)

if os.path.isdir(fr"{DEPS_INSTALL_DIR}\lib\qml"):
    print("ERROR: some of Deps' QML modules were installed in an incorrect location")
    print(fr"    actual path: {DEPS_INSTALL_DIR}\lib\qml")
    print(fr"    expected path: {DEPS_INSTALL_DIR}\qml")
    exit(103)

# windeployqt
if useQt6Build:
    # NOTE: we don't pass `--release` option to activate autodetection of the build type
    #       (which will effectively accept any kind of the binaries unless on MSVC)
    subprocess.run(["windeployqt.exe", *QMLDIR_ARGS,
                    "-gui", "-core", "-core5compat", "-openglwidgets", "-svgwidgets", "-opengl",
                    "-concurrent", "-network", "-printsupport", "-svg",
                    "-xml", "-sql", "-qml", "-quick", "-quickwidgets", "-verbose", "2",
                    f"{pkg_root}\\bin\\krita.exe", f"{pkg_root}\\bin\\krita.dll"], check=True)
else:
    subprocess.run(["windeployqt.exe", *QMLDIR_ARGS, "--release", "-gui", "-core", "-concurrent", "-network", "-printsupport", "-svg",
                    "-xml", "-sql", "-qml", "-quick", "-quickwidgets", f"{pkg_root}\\bin\\krita.exe", f"{pkg_root}\\bin\\krita.dll"], check=True)

# ffmpeg
if os.path.exists(f"{DEPS_INSTALL_DIR}\\bin\\ffmpeg.exe"):
    shutil.copy(f"{DEPS_INSTALL_DIR}\\bin\\ffmpeg.exe", f"{pkg_root}\\bin")
    shutil.copy(f"{DEPS_INSTALL_DIR}\\bin\\ffprobe.exe", f"{pkg_root}\\bin")
    if os.path.exists(f"{DEPS_INSTALL_DIR}\\bin\\ffmpeg_LICENSE.txt"):
        shutil.copy(f"{DEPS_INSTALL_DIR}\\bin\\ffmpeg_LICENSE.txt",
                    f"{pkg_root}\\bin")
    if os.path.exists(f"{DEPS_INSTALL_DIR}\\bin\\ffmpeg_README.txt"):
        shutil.copy(f"{DEPS_INSTALL_DIR}\\bin\\ffmpeg_README.txt",
                    f"{pkg_root}\\bin")

# Copy embedded Python
subprocess.run(["xcopy", "/S", "/Y", "/I",
               f"{DEPS_INSTALL_DIR}\\python", f"{pkg_root}\\python"], check=True)
if os.path.exists(f"{pkg_root}\\python\\python.exe"):
    os.remove(f"{pkg_root}\\python\\python.exe")
if os.path.exists(f"{pkg_root}\\python\\pythonw.exe"):
    os.remove(f"{pkg_root}\\python\\pythonw.exe")

# Remove Python cache files
for d in os.walk(pkg_root):
    pycache = f"{d[0]}\\__pycache__"
    if os.path.isdir(pycache):
        print(f"Deleting Python cache {pycache}")
        shutil.rmtree(pycache)

if os.path.exists(f"{pkg_root}\\lib\\site-packages"):
    print(f"Deleting unnecessary Python packages")
    for f in glob.glob(f"{pkg_root}\\lib\\site-packages\\packaging*"):
        shutil.rmtree(f)
    for f in glob.glob(f"{pkg_root}\\lib\\site-packages\\pip*"):
        shutil.rmtree(f)
    for f in glob.glob(f"{pkg_root}\\lib\\site-packages\\ply*"):
        shutil.rmtree(f)
    for f in glob.glob(f"{pkg_root}\\lib\\site-packages\\pyparsing*"):
        shutil.rmtree(f)
    for f in glob.glob(f"{pkg_root}\\lib\\site-packages\\PyQt_builder*"):
        shutil.rmtree(f)
    for f in glob.glob(f"{pkg_root}\\lib\\site-packages\\setuptools.pth"):
        os.remove(f)
    for f in glob.glob(f"{pkg_root}\\lib\\site-packages\\setuptools*"):
        shutil.rmtree(f)
    for f in glob.glob(f"{pkg_root}\\lib\\site-packages\\sip*"):
        shutil.rmtree(f)
    for f in glob.glob(f"{pkg_root}\\lib\\site-packages\\toml*"):
        shutil.rmtree(f)
    for f in glob.glob(f"{pkg_root}\\lib\\site-packages\\meson*"):
        shutil.rmtree(f)
    for f in glob.glob(f"{pkg_root}\\lib\\site-packages\\easy-install.pth"):
        os.remove(f)

if not os.environ.get('KRITACI_SKIP_SPLIT_DEBUG', '0').lower() in ['true', '1', 'on']:
    print("\nSplitting debug info from binaries...")

    def split_debug(arg1, arg2):
        print(f"Splitting debug info of {arg2}")
        commandToRun = f"objcopy --only-keep-debug {arg1} {arg1}.debug"
        try:
            subprocess.check_call(commandToRun, stdout=sys.stdout, stderr=sys.stderr, shell=True)
        except subprocess.CalledProcessError as status:
            return status.returncode
        # If the debug file is small enough then consider there being no debug info.
        # Discard these files since they somehow make gdb crash.
        if os.path.getsize(f"{arg1}.debug") <= 2048:
            print(f"Discarding {arg2}.debug")
            os.remove(f"{arg1}.debug")
            return 0
        debugDir = os.path.dirname(arg1)+".debug"
        if not os.path.isdir(debugDir):
            os.mkdir(debugDir)
        shutil.move(f"{arg1}.debug", debugDir+"\\")
        commandToRun = f"strip --strip-debug {arg1}"
        subprocess.check_call(commandToRun, stdout=sys.stdout, stderr=sys.stderr, shell=True)
        # Add debuglink
        # FIXME: There is a problem with gdb that cause it to output this warning
        # FIXME: "warning: section .gnu_debuglink not found in xxx.debug"
        # FIXME: I tried adding a link to itself but this kills drmingw :(
        commandToRun = fr'objcopy --add-gnu-debuglink="{debugDir}\{os.path.basename(arg1)}.debug" {arg1}'
        try:
            subprocess.check_call(commandToRun, stdout=sys.stdout, stderr=sys.stderr, shell=True)
        except subprocess.CalledProcessError as status:
            return status.returncode
        return 0

    split_debug(fr"{pkg_root}\bin\krita.exe", r"bin\krita.exe")
    split_debug(fr"{pkg_root}\bin\krita.com", r"bin\krita.com")
    split_debug(fr"{pkg_root}\bin\kritarunner.exe", r"bin\kritarunner.exe")
    split_debug(fr"{pkg_root}\bin\kritarunner.com", r"bin\kritarunner.com")
    # Find all DLLs
    files = glob.glob(fr"{pkg_root}\**\*.dll", recursive=True)
    for f in files:
        split_debug(f, os.path.relpath(f, pkg_root))
    # Find all Python native modules
    files = glob.glob(fr"{pkg_root}\share\krita\pykrita\**\*.pyd", recursive=True)
    for f in files:
        split_debug(f, os.path.relpath(f, pkg_root))
    files = glob.glob(fr"{pkg_root}\lib\krita-python-libs\**\*.pyd", recursive=True)
    for f in files:
        split_debug(f, os.path.relpath(f, pkg_root))
    files = glob.glob(fr"{pkg_root}\lib\site-packages\**\*.pyd", recursive=True)
    for f in files:
        split_debug(f, os.path.relpath(f, pkg_root))


if args.pre_zip_hook:
    print("Running pre-zip hook...")
    if args.pre_zip_hook.endswith('.cmd'):
        subprocess.run(["cmd", "/c", args.pre_zip_hook,
                       f"{pkg_root}\\"], stdout=sys.stdout, stderr=sys.stderr, shell=True, check=True)
    elif args.pre_zip_hook.endswith('.py'):
        subprocess.run([sys.executable, "-u", args.pre_zip_hook,
                       f"{pkg_root}\\"], stdout=sys.stdout, stderr=sys.stderr, shell=True, check=True)
    else:
        warnings.warn("ERROR: pre-zip hook has unknown format!")
        sys.exit(102)

print("\nPackaging stripped binaries...")
subprocess.run([os.environ["SEVENZIP_EXE"], "a", "-tzip",
               f"{pkg_name}.zip", f"{pkg_root}\\", "-xr!*.debug"], check=True)
print("--------\n")

if not os.environ.get('KRITACI_SKIP_DEBUG_PACKAGE', '0').lower() in ['true', '1', 'on']:
    print("Packaging debug info...")
    # (note that the top-level package dir is not included)
    subprocess.run([os.environ["SEVENZIP_EXE"], "a", "-tzip",
                f"{pkg_name}-dbg.zip", "-r", f"{pkg_root}\\*.debug"], check=True)
    print("--------\n")

print("\n")
print(f"Krita packaged as {pkg_name}.zip")
if os.path.isfile(f"{pkg_name}-dbg.zip"):
    print(f"Debug info packaged as {pkg_name}-dbg.zip")
print(f"Packaging dir is {pkg_root}")
print("NOTE: Do not create installer with packaging dir. Extract from")
print(f"      {pkg_name}.zip instead")
print("and do _not_ run krita inside the extracted directory because it will")
print("       create extra unnecessary files.\n")
print("Please remember to actually test the package before releasing it.\n")
