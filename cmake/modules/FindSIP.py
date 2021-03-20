# FindSIP.py
#
# SPDX-FileCopyrightText: 2007 Simon Edwards <simon@simonzone.com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

try:
    import sipbuild
except ImportError:  # Code for SIP v4
    import sipconfig

    sipcfg = sipconfig.Configuration()
    print("sip_version:%06.0x" % sipcfg.sip_version)
    print("sip_version_str:%s" % sipcfg.sip_version_str)
    print("sip_bin:%s" % sipcfg.sip_bin)
    print("default_sip_dir:%s" % sipcfg.default_sip_dir)
    print("sip_inc_dir:%s" % sipcfg.sip_inc_dir)
else:  # Code for SIP v5
    print("sip_version:%06.0x" % sipbuild.version.SIP_VERSION)
    print("sip_version_str:%s" % sipbuild.version.SIP_VERSION_STR)

    import shutil
    print("sip_bin:%s" % shutil.which("sip5"))

    from distutils.sysconfig import get_python_lib
    python_modules_dir = get_python_lib(plat_specific=1)
    print("default_sip_dir:%s" % python_modules_dir)
