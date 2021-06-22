# FindSIP.py
#
# SPDX-FileCopyrightText: 2007 Simon Edwards <simon@simonzone.com>
# SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
#
# SPDX-License-Identifier: BSD-3-Clause
#

import os

def osAwareExecutable(filename):
    return filename if os.name != 'nt' else filename + ".exe"

def osAwareSuffix():
    return os.path.join("lib", "python3.8", "site-packages","sipconfig.py") if os.name != 'nt' else os.path.join("lib","krita-python-libs","sipconfig.py")

try:
    import sipconfig

    sipcfg = sipconfig.Configuration()
    sip_version = sipcfg.sip_version
    sip_version_str = sipcfg.sip_version_str
    sip_bin = sipcfg.sip_bin
    default_sip_dir = sipcfg.default_sip_dir
    sip_inc_dir = sipcfg.sip_inc_dir
    if not os.path.isfile(osAwareExecutable(sip_bin)):
        # Relocated deps, attempt to fix the paths...
        sipconfig_path = os.path.abspath(sipconfig.__file__)
        sipconfig_expected_suffix = osAwareSuffix()
        if sipconfig_path.endswith(sipconfig_expected_suffix):
            deps_prefix = sipconfig_path[:-len(sipconfig_expected_suffix)]
            sip_bin_expected_suffix = os.path.join("bin","sip")
            if sip_bin.endswith(sip_bin_expected_suffix):
                orig_prefix = sip_bin[:-len(sip_bin_expected_suffix)]
                if os.path.isfile(osAwareExecutable(sip_bin.replace(orig_prefix, deps_prefix, 1))):
                    sip_bin = sip_bin.replace(orig_prefix, deps_prefix, 1)
                    default_sip_dir = default_sip_dir.replace(orig_prefix, deps_prefix, 1)
                    sip_inc_dir = sip_inc_dir.replace(orig_prefix, deps_prefix, 1)
    print("sip_version:%06.0x" % sip_version)
    print("sip_version_str:%s" % sip_version_str)
    print("sip_bin:%s" % sip_bin)
    print("default_sip_dir:%s" % default_sip_dir)
    print("sip_inc_dir:%s" % sip_inc_dir)
except ImportError:  # Code for SIP v5+
    import sipbuild

    print("sip_version:%06.0x" % sipbuild.version.SIP_VERSION)
    print("sip_version_str:%s" % sipbuild.version.SIP_VERSION_STR)

    import shutil

    # sip v5 and higher need to invoke sip-build
    print("sip_bin:%s" % shutil.which("sip-build"))
