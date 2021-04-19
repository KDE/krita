# FindSIP.py
#
# SPDX-FileCopyrightText: 2007 Simon Edwards <simon@simonzone.com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

import os

try:
    import sipbuild
except ImportError:  # Code for SIP v4
    import sipconfig

    sipcfg = sipconfig.Configuration()
    sip_version = sipcfg.sip_version
    sip_version_str = sipcfg.sip_version_str
    sip_bin = sipcfg.sip_bin
    default_sip_dir = sipcfg.default_sip_dir
    sip_inc_dir = sipcfg.sip_inc_dir
    if os.name == 'nt' and not os.path.isfile(sip_bin + ".exe"):
        # Relocated deps, attempt to fix the paths...
        sipconfig_path = os.path.abspath(sipconfig.__file__)
        sipconfig_expected_suffix = r"\lib\krita-python-libs\sipconfig.py"
        if sipconfig_path.endswith(sipconfig_expected_suffix):
            deps_prefix = sipconfig_path[:-len(sipconfig_expected_suffix)]
            sip_bin_expected_suffix = r"\bin\sip"
            if sip_bin.endswith(sip_bin_expected_suffix):
                orig_prefix = sip_bin[:-len(sip_bin_expected_suffix)]
                if os.path.isfile(sip_bin.replace(orig_prefix, deps_prefix, 1) + ".exe"):
                    sip_bin = sip_bin.replace(orig_prefix, deps_prefix, 1)
                    default_sip_dir = default_sip_dir.replace(orig_prefix, deps_prefix, 1)
                    sip_inc_dir = sip_inc_dir.replace(orig_prefix, deps_prefix, 1)
    print("sip_version:%06.0x" % sip_version)
    print("sip_version_str:%s" % sip_version_str)
    print("sip_bin:%s" % sip_bin)
    print("default_sip_dir:%s" % default_sip_dir)
    print("sip_inc_dir:%s" % sip_inc_dir)
else:  # Code for SIP v5
    print("sip_version:%06.0x" % sipbuild.version.SIP_VERSION)
    print("sip_version_str:%s" % sipbuild.version.SIP_VERSION_STR)

    import shutil
    print("sip_bin:%s" % shutil.which("sip5"))

    from distutils.sysconfig import get_python_lib
    python_modules_dir = get_python_lib(plat_specific=1)
    print("default_sip_dir:%s" % python_modules_dir)
