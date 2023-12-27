# SPDX-FileCopyright: 2021 Alvin Wong <alvin@alvinhc.com>
# SPDX-FileCopyright: 2022 L. E. Segovia <amy@amyspark.me>
# SPDX-License-Ref: GPL-2.0-or-later

# Use the debian/pkgsrc hook to patch the MSVCRT lookup out.

from distutils import cygwinccompiler
import sys

def _get_msvcr_replacement():
    # So, the original `get_msvcr` function is supposed to return the name
    # of the specific version of MS C runtime so that it can be (?) copied
   # over. But we don't really need to do it since we have he packaging
    # script to take care of that.
 return []


if not callable(cygwinccompiler.get_msvcr):
 raise RuntimeError(
 "distutils.cygwinccompiler.get_msvcr is not a function, which is unexpected")

# print("[*] Krita is patching cygwinccompiler.get_msvcr... ({})".format(cygwinccompiler.__file__))

# HACK: Replace the function to apply our hack...
cygwinccompiler.get_msvcr = _get_msvcr_replacement

# print("[*] Check: {} {}".format(cygwinccompiler.get_msvcr,
#       cygwinccompiler.get_msvcr()))


# distutils's cygwinccompiler module contains some very old version checking
# code that had only been removed upstream recently in [1]. This includes a
# version check for `ld`. The llvm-mingw toolchain only contains `ld.lld.exe`
# as an executable but not `ld.exe`, which breaks this check. Here we just
# monkey patch this check to return some modern version for `ld` to make the
# check not fail.
# [1]: https://github.com/pypa/distutils/commit/221a2f2888aa1a48887003c5afa10c1d18ae3a92

_original_get_versions = cygwinccompiler.get_versions

def _get_versions_replacement():
    gcc_ver, ld_ver, dllwrap_ver = _original_get_versions()
    if ld_ver is None:
        ld_ver = "2.37"
    return gcc_ver, ld_ver, dllwrap_ver

if callable(cygwinccompiler.get_versions):
    # print("[*] Krita is patching cygwinccompiler.get_versions... ({})".format(cygwinccompiler.__file__))
    cygwinccompiler.get_versions = _get_versions_replacement


# HACK: Override distutils.cygwinccompiler lookup
# https://github.com/pypa/setuptools/blob/298f5a6368397977ee09cb0b39d5f76aa544b048/setuptools/_distutils/ccompiler.py#L1024
# print("[*] Freezing distutils.cygwinccompiler module")

sys.modules["distutils.cygwinccompiler"] = cygwinccompiler
