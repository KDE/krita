# SPDX-FileCopyrightText: 2014 Simon Edwards <simon@simonzone.com>
# SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
#
# SPDX-License-Identifier: BSD-3-Clause
#

import re
import sys
import os
from distutils.sysconfig import get_python_lib

try:
    # On Windows and Python 3.8+ python doesn't load module DLL's
    # from the current PATH environment, therefore we should add
    # path to Qt's DLL's manually. This variable is passed from
    # FindPyQt5.cmake
    for path in os.environ['PYTHONDLLPATH'].split(';'):
        os.add_dll_directory(path)
except:
    pass

import PyQt5.QtCore

print("pyqt_version:%06.0x" % PyQt5.QtCore.PYQT_VERSION)
print("pyqt_version_str:%s" % PyQt5.QtCore.PYQT_VERSION_STR)

pyqt_version_tag = ""
in_t = False
pyqt_config_list = PyQt5.QtCore.PYQT_CONFIGURATION["sip_flags"].split(' ')
for item in pyqt_config_list:
    if item == "-t":
        in_t = True
    elif in_t:
        if item.startswith("Qt_5"):
            pyqt_version_tag = item
    else:
        in_t = False
print("pyqt_version_tag:%s" % pyqt_version_tag)

try:
    index_n = pyqt_config_list.index('-n')
    pyqt_sip_name = pyqt_config_list[index_n + 1]
    print("pyqt_sip_name:%s" % pyqt_sip_name)
except ValueError:
    pass

# If we use a system install with a custom PYTHONPATH, PyQt5
# may not be within the system tree.
pyqt5_dir = os.path.dirname(PyQt5.__file__)
lib_site_packages = get_python_lib(plat_specific=1)
# Check this first.
if pyqt5_dir.startswith(lib_site_packages):
    pyqt_sip_dir = os.path.join(get_python_lib(plat_specific=1), "PyQt5", "bindings")
else:
    # If so, change it.
    pyqt_sip_dir = os.path.join(pyqt5_dir, "bindings")

if not os.path.exists(pyqt_sip_dir):  # Fallback for older PyQt5/SIP
    pyqt_sip_dir = os.path.join(sys.prefix, "share", "sip", "PyQt5")
print("pyqt_sip_dir:%s" % pyqt_sip_dir)

print("pyqt_sip_flags:%s" % PyQt5.QtCore.PYQT_CONFIGURATION["sip_flags"])

tags = re.findall(r"-t ([^\s]+)", PyQt5.QtCore.PYQT_CONFIGURATION["sip_flags"])
print("pyqt_sip_tags:%s" % ",".join(tags))
