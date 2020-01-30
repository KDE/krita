#.rst:
# FindLibExiv2
# ------------
#
# Try to find the Exiv2 library.
#
# This will define the following variables:
#
# ``LibExiv2_FOUND``
#     System has LibExiv2.
#
# ``LibExiv2_VERSION``
#     The version of LibExiv2.
#
# ``LibExiv2_INCLUDE_DIRS``
#     This should be passed to target_include_directories() if
#     the target is not used for linking.
#
# ``LibExiv2_LIBRARIES``
#     The LibExiv2 library.
#     This can be passed to target_link_libraries() instead of
#     the ``LibExiv2::LibExiv2`` target
#
# If ``LibExiv2_FOUND`` is TRUE, the following imported target
# will be available:
#
# ``LibExiv2::LibExiv2``
#     The Exiv2 library
#
# Since 5.53.0.
#
#=============================================================================
# Copyright (c) 2018, Christophe Giboudeaux, <christophe@krop.fr>
# Copyright (c) 2010, Alexander Neundorf, <neundorf@kde.org>
# Copyright (c) 2008, Gilles Caulier, <caulier.gilles@gmail.com>
#
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#=============================================================================

find_package(PkgConfig QUIET)
pkg_check_modules(PC_EXIV2 QUIET exiv2)

find_path(LibExiv2_INCLUDE_DIRS NAMES exiv2/exif.hpp
    HINTS ${PC_EXIV2_INCLUDEDIR}
)

find_library(LibExiv2_LIBRARIES NAMES exiv2 libexiv2
    HINTS ${PC_EXIV2_LIBRARY_DIRS}
)

set(LibExiv2_VERSION ${PC_EXIV2_VERSION})

if(NOT LibExiv2_VERSION AND DEFINED LibExiv2_INCLUDE_DIRS)
    # With exiv >= 0.27, the version #defines are in exv_conf.h instead of version.hpp
    foreach(_exiv2_version_file "version.hpp" "exv_conf.h")
        if(EXISTS "${LibExiv2_INCLUDE_DIRS}/exiv2/${_exiv2_version_file}")
            file(READ "${LibExiv2_INCLUDE_DIRS}/exiv2/${_exiv2_version_file}" _exiv_version_file_content)
            string(REGEX MATCH "#define EXIV2_MAJOR_VERSION[ ]+\\([0-9]+\\)" EXIV2_MAJOR_VERSION_MATCH ${_exiv_version_file_content})
            string(REGEX MATCH "#define EXIV2_MINOR_VERSION[ ]+\\([0-9]+\\)" EXIV2_MINOR_VERSION_MATCH ${_exiv_version_file_content})
            string(REGEX MATCH "#define EXIV2_PATCH_VERSION[ ]+\\([0-9]+\\)" EXIV2_PATCH_VERSION_MATCH ${_exiv_version_file_content})
            if(EXIV2_MAJOR_VERSION_MATCH)
                string(REGEX REPLACE ".*_MAJOR_VERSION[ ]+\\((.*)\\)" "\\1" EXIV2_MAJOR_VERSION ${EXIV2_MAJOR_VERSION_MATCH})
                string(REGEX REPLACE ".*_MINOR_VERSION[ ]+\\((.*)\\)" "\\1" EXIV2_MINOR_VERSION ${EXIV2_MINOR_VERSION_MATCH})
                string(REGEX REPLACE ".*_PATCH_VERSION[ ]+\\((.*)\\)" "\\1"  EXIV2_PATCH_VERSION  ${EXIV2_PATCH_VERSION_MATCH})
            endif()
        endif()
    endforeach()

    set(LibExiv2_VERSION "${EXIV2_MAJOR_VERSION}.${EXIV2_MINOR_VERSION}.${EXIV2_PATCH_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibExiv2
    FOUND_VAR LibExiv2_FOUND
    REQUIRED_VARS  LibExiv2_LIBRARIES LibExiv2_INCLUDE_DIRS
    VERSION_VAR  LibExiv2_VERSION
)

mark_as_advanced(LibExiv2_INCLUDE_DIRS LibExiv2_LIBRARIES)

if(LibExiv2_FOUND AND NOT TARGET LibExiv2::LibExiv2)
    add_library(LibExiv2::LibExiv2 UNKNOWN IMPORTED)
    set_target_properties(LibExiv2::LibExiv2 PROPERTIES
        IMPORTED_LOCATION "${LibExiv2_LIBRARIES}"
        INTERFACE_INCLUDE_DIRECTORIES "${LibExiv2_INCLUDE_DIRS}"
    )
endif()

include(FeatureSummary)
set_package_properties(LibExiv2 PROPERTIES
    URL "http://www.exiv2.org"
    DESCRIPTION "Image metadata support"
)
