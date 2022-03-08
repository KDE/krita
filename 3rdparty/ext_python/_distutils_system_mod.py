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

print("[*] Krita is patching cygwinccompiler.get_msvcr... ({})".format(cygwinccompiler.__file__))

# HACK: Replace the function to apply our hack...
cygwinccompiler.get_msvcr = _get_msvcr_replacement

print("[*] Check: {} {}".format(cygwinccompiler.get_msvcr,
      cygwinccompiler.get_msvcr()))


# HACK: Override distutils.cygwinccompiler lookup
# https://github.com/pypa/setuptools/blob/298f5a6368397977ee09cb0b39d5f76aa544b048/setuptools/_distutils/ccompiler.py#L1024
print("[*] Freezing distutils.cygwinccompiler module")

sys.modules["distutils.cygwinccompiler"] = cygwinccompiler
