# SPDX-FileCopyrightText: 2014 Simon Edwards <simon@simonzone.com>
# SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
#
# SPDX-License-Identifier: BSD-3-Clause
#

import re
import sys
import os
import sysconfig

try:
    # On Windows and Python 3.8+ python doesn't load module DLL's
    # from the current PATH environment, therefore we should add
    # path to Qt's DLL's manually. This variable is passed from
    # FindPyQt6.cmake
    for path in os.environ['PYTHONDLLPATH'].split(';'):
        os.add_dll_directory(path)
except:
    pass

import PyQt6.QtCore

print("pyqt_version:%06.0x" % PyQt6.QtCore.PYQT_VERSION)
print("pyqt_version_str:%s" % PyQt6.QtCore.PYQT_VERSION_STR)

pyqt_version_tag = ""
in_t = False

# If we use a system install with a custom PYTHONPATH, PyQt6
# may not be within the system tree.
pyqt6_dir = os.path.dirname(PyQt6.__file__)
lib_site_packages = sysconfig.get_path('platlib')
# Check this first.
if pyqt6_dir.startswith(lib_site_packages):
    pyqt_sip_dir = os.path.join(sysconfig.get_path('platlib'), "PyQt6", "bindings")
else:
    # If so, change it.
    pyqt_sip_dir = os.path.join(pyqt6_dir, "bindings")

print("pyqt_sip_dir:%s" % pyqt_sip_dir)
