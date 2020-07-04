#
# Copyright 2016 Pixar
#
# Licensed under the Apache License, Version 2.0 (the "Apache License")
# with the following modification; you may not use this file except in
# compliance with the Apache License and the following modification to it:
# Section 6. Trademarks. is deleted and replaced with:
#
# 6. Trademarks. This License does not grant permission to use the trade
#    names, trademarks, service marks, or product names of the Licensor
#    and its affiliates, except as required to comply with Section 4(c) of
#    the License and to reproduce the content of the NOTICE file.
#
# You may obtain a copy of the Apache License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the Apache License with the above modification is
# distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied. See the Apache License for the specific
# language governing permissions and limitations under the Apache License.
#

find_path(OPENEXR_INCLUDE_DIR
    OpenEXR/half.h
HINTS
    "${OPENEXR_LOCATION}"
    "$ENV{OPENEXR_LOCATION}"
PATH_SUFFIXES
    include/
DOC
    "OpenEXR headers path"
)

if(OPENEXR_INCLUDE_DIR)
  set(openexr_config_file "${OPENEXR_INCLUDE_DIR}/OpenEXR/OpenEXRConfig.h")
  if(EXISTS ${openexr_config_file})
      file(STRINGS
           ${openexr_config_file}
           TMP
           REGEX "#define OPENEXR_VERSION_STRING.*$")
      string(REGEX MATCHALL "[0-9.]+" OPENEXR_VERSION ${TMP})

      file(STRINGS
           ${openexr_config_file}
           TMP
           REGEX "#define OPENEXR_VERSION_MAJOR.*$")
      string(REGEX MATCHALL "[0-9]" OPENEXR_MAJOR_VERSION ${TMP})

      file(STRINGS
           ${openexr_config_file}
           TMP
           REGEX "#define OPENEXR_VERSION_MINOR.*$")
      string(REGEX MATCHALL "[0-9]" OPENEXR_MINOR_VERSION ${TMP})
  endif()
endif()

foreach(OPENEXR_LIB
    Half
    Iex
    Imath
    IlmImf
    IlmThread
    )

    # OpenEXR libraries may be suffixed with the version number, so we search
    # using both versioned and unversioned names.
    find_library(OPENEXR_${OPENEXR_LIB}_LIBRARY
        NAMES
            ${OPENEXR_LIB}-${OPENEXR_MAJOR_VERSION}_${OPENEXR_MINOR_VERSION}
            ${OPENEXR_LIB}
        HINTS
            "${OPENEXR_LOCATION}"
            "$ENV{OPENEXR_LOCATION}"
        PATH_SUFFIXES
            lib/
        DOC
            "OPENEXR's ${OPENEXR_LIB} library path"
    )

    if(OPENEXR_${OPENEXR_LIB}_LIBRARY)
        list(APPEND OPENEXR_LIBRARIES ${OPENEXR_${OPENEXR_LIB}_LIBRARY})
    endif()
endforeach(OPENEXR_LIB)

# So #include <half.h> works
list(APPEND OPENEXR_INCLUDE_DIRS ${OPENEXR_INCLUDE_DIR})
list(APPEND OPENEXR_INCLUDE_DIRS ${OPENEXR_INCLUDE_DIR}/OpenEXR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenEXR
    REQUIRED_VARS
        OPENEXR_INCLUDE_DIRS
        OPENEXR_LIBRARIES
    VERSION_VAR
        OPENEXR_VERSION
)
