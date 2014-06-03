# Locate the Vc template library. Vc can be found at http://gitorious.org/Vc/
#
# Copyright 2009-2012   Matthias Kretz <kretz@kde.org>
#
# This file is meant to be copied into projects that want to use Vc. It will
# search for VcConfig.cmake, which ships with Vc and will provide up-to-date
# buildsystem changes. Thus there should not be any need to update FindVc.cmake
# again after you integrated it into your project.
#
# This module defines the following variables:
# Vc_FOUND
# Vc_INCLUDE_DIR
# Vc_LIBRARIES
# Vc_DEFINITIONS
# Vc_VERSION_MAJOR
# Vc_VERSION_MINOR
# Vc_VERSION_PATCH
# Vc_VERSION
# Vc_VERSION_STRING
# Vc_INSTALL_DIR
# Vc_LIB_DIR
# Vc_CMAKE_MODULES_DIR
#
# The following two variables are set according to the compiler used. Feel free
# to use them to skip whole compilation units.
# Vc_SSE_INTRINSICS_BROKEN
# Vc_AVX_INTRINSICS_BROKEN

find_package(Vc ${Vc_FIND_VERSION} QUIET NO_MODULE PATHS $ENV{HOME} /opt/Vc)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Vc CONFIG_MODE)
