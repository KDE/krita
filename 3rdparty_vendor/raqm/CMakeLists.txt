# SPDX-FileCopyrightText: 2023 Alvin Wong <alvin@alvinhc.com>
# SPDX-License-Identifier: BSD-2-Clause

#[=======================================================================[

This CMake file builds a patched libraqm as a static library to be linked from
Krita (libkritaflake). We use FetchContent to download a copy of the unmodified
libraqm source and patch it locally.

FetchContent provides a way to use existing source instead of a downloaded copy.
By setting `FETCHCONTENT_SOURCE_DIR_LIBRAQM_SRC` to the path of a local copy of
the patched libraqm source, you can skip the download step, and you will be
able to modify the local copy of the libraqm source to test out changes.

#]=======================================================================]


project(libraqm
    VERSION 0.10.1.1
    DESCRIPTION "A library for complex text layout"
    HOMEPAGE_URL "https://github.com/HOST-Oman/libraqm"
    LANGUAGES C
)

##
## Check for FriBidi
##
find_package(FriBidi 1.0.6 REQUIRED)
set_package_properties(FriBidi PROPERTIES
    DESCRIPTION "GNU FriBidi"
    URL "https://github.com/fribidi/fribidi"
    TYPE REQUIRED
    PURPOSE "Needed by libraqm to perform bidirectional text layout."
)


include(FetchContent)
if(NOT DEFINED PATCH_COMMAND)
    if(WIN32)
        set(PATCH_COMMAND patch --binary)
    else()
        set(PATCH_COMMAND patch)
    endif()
endif()


if (USE_EXTERNAL_RAQM)
    set(RAQM_FETCHCONTENT_ARGS
        URL https://github.com/HOST-Oman/libraqm/releases/download/v0.10.1/raqm-0.10.1.tar.xz
        # HACK: Whenever you change this file to add patch commands, it causes
        # CMake to rerun the patch commands on the already-patched source, which
        # results in a failure. Changing the following fake URL tricks CMake
        # into re-extracting the source so the patch commands can succeed.
        http://:0/hack_to_trigger_re-extract-source/20230914
        URL_HASH SHA256=4d76a358358d67c5945684f2f10b3b08fb80e924371bf3ebf8b15cd2e321d05d
        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Add-arbitrary-run-break-function.patch
              COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0002-Fix-Unicode-codepoint-conversion-from-UTF-16.patch)
else ()
    set(RAQM_FETCHCONTENT_ARGS SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/raqm-0.10.1)
endif ()

FetchContent_Declare(
    libraqm_src
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    ${RAQM_FETCHCONTENT_ARGS}
)

FetchContent_MakeAvailable(libraqm_src)
if(NOT libraqm_src_POPULATED)
    message(FATAL_ERROR "Missing libraqm source.")
endif()


set(CMAKE_AUTOMOC OFF)

set(RAQM_VERSION_MAJOR ${libraqm_VERSION_MAJOR})
set(RAQM_VERSION_MINOR ${libraqm_VERSION_MINOR})
set(RAQM_VERSION_MICRO ${libraqm_VERSION_PATCH})
set(RAQM_VERSION ${libraqm_VERSION})
configure_file(${libraqm_src_SOURCE_DIR}/src/raqm-version.h.in ${CMAKE_CURRENT_BINARY_DIR}/raqm-version.h @ONLY)

set(libraqm_SRC
    ${libraqm_src_SOURCE_DIR}/src/raqm.c
    ${libraqm_src_SOURCE_DIR}/src/raqm.h
)

add_library(libraqm STATIC ${libraqm_SRC})
set_property(TARGET libraqm PROPERTY C_STANDARD 99)

target_include_directories(libraqm INTERFACE ${libraqm_src_SOURCE_DIR}/src)
target_include_directories(libraqm PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(libraqm
    PUBLIC
        Freetype::Freetype
    PRIVATE
        HarfBuzz::HarfBuzz
        FriBidi::FriBidi
)

add_library(Raqm::Raqm ALIAS libraqm)
