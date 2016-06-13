# Copyright (c) 2014, Simon Edwards <simon@simonzone.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

import PyQt5.Qt
import sys
import os.path

print("pyqt_version:%06.0x" % PyQt5.Qt.PYQT_VERSION)
print("pyqt_version_str:%s" % PyQt5.Qt.PYQT_VERSION_STR)

pyqt_version_tag = ""
in_t = False
for item in PyQt5.Qt.PYQT_CONFIGURATION["sip_flags"].split(' '):
    if item=="-t":
        in_t = True
    elif in_t:
        if item.startswith("Qt_5"):
            pyqt_version_tag = item
    else:
        in_t = False
print("pyqt_version_tag:%s" % pyqt_version_tag)

# FIXME This next line is just a little bit too crude.
pyqt_sip_dir = os.path.join(sys.prefix, "share", "sip", "PyQt5")
print("pyqt_sip_dir:%s" % pyqt_sip_dir)

print("pyqt_sip_flags:%s" % PyQt5.Qt.PYQT_CONFIGURATION["sip_flags"])
