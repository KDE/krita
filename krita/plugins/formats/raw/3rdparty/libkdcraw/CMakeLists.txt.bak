#
# Copyright (c) 2010-2015, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

project(libkdcraw)

message(STATUS "----------------------------------------------------------------------------------")
message(STATUS "Starting CMake configuration for: ${PROJECT_NAME}")

set(CMAKE_MIN_VERSION   "2.8.12")
set(ECM_MIN_VERSION     "1.1.0")
set(KF5_VERSION         "5.1.0")
set(REQUIRED_QT_VERSION "5.2.0")
set(LIBRAW_MIN_VERSION  "0.16")

cmake_minimum_required(VERSION ${CMAKE_MIN_VERSION})

# ==================================================================================================
# Information to update before to release this library.

# Library version history:
# API      ABI
# 0.1.0 => 0.1.0  (KDE3)
# 0.1.1 => 1.0.0  (KDE3)
# 0.1.2 => 2.0.0  (KDE3)
# 0.1.3 => 2.1.0  (KDE3)
# 0.1.4 => 3.0.0  (KDE3)
# 0.1.5 => 4.0.0  (KDE3)
# 0.2.0 => 5.0.0  (released with KDE 4.1.0)
# 0.3.0 => 6.0.0  (released with KDE 4.1.2)
# 0.4.0 => 7.0.0  (using libRaw-Released with KDE 4.2.0)
# 0.5.0 => 7.1.0  (Released with KDE 4.3.0)
# 1.0.0 => 8.0.0  (Released with KDE 4.4.0)
# 1.1.0 => 8.1.0  (Released with KDE 4.4.2)
# 1.2.0 => 9.0.0  (Released with KDE 4.5.0)
# 1.3.0 => 10.0.0 (Released with KDE 4.6.0)
# 2.0.0 => 20.0.0 (Released with KDE 4.7.0)
# 2.0.1 => 20.0.1 (Released with KDE 4.8.0  - Introducing new deprecated members due to use libraw 0.14.x)
# 2.1.0 => 21.0.0 (Released with KDE 4.8.1  - Remove deprecated members)
# 2.2.0 => 22.0.0 (Released with KDE 4.10.0 - Including RawSpeed Codec)
# 2.3.0 => 22.1.0 (Released with KDE 4.11.0 - Including Coverity Report fixes)
# 2.3.1 => 22.1.1 (Released with KDE 4.11.2 - Including settings widget fixes)
# 2.4.0 => 23.0.2 (Released with KDE 4.12.0 - Drop internal Libraw source code + new methods to get thumb and preview from QBuffer)
# 5.0.0 => 30.0.0 (Released with KDE 5.x)

# Library API version
SET(KDCRAW_LIB_MAJOR_VERSION "5")
SET(KDCRAW_LIB_MINOR_VERSION "0")
SET(KDCRAW_LIB_PATCH_VERSION "0")

# Library ABI version used by linker.
# For details : http://www.gnu.org/software/libtool/manual/libtool.html#Updating-version-info
SET(KDCRAW_LIB_SO_CUR_VERSION "30")
SET(KDCRAW_LIB_SO_REV_VERSION "0")
SET(KDCRAW_LIB_SO_AGE_VERSION "0")

SET(LIBKDCRAW_LIB_VERSION "${KDCRAW_LIB_MAJOR_VERSION}.${KDCRAW_LIB_MINOR_VERSION}.${KDCRAW_LIB_PATCH_VERSION}")
SET(LIBDCRAW_SO_VERSION   "${KDCRAW_LIB_SO_CUR_VERSION}.${KDCRAW_LIB_SO_REV_VERSION}.${KDCRAW_LIB_SO_AGE_VERSION}")

############## ECM setup ######################

find_package(ECM ${ECM_MIN_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${libkdcraw_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})

include(ECMGenerateHeaders)
include(ECMGeneratePriFile)
include(ECMPackageConfigHelpers)
include(ECMSetupVersion)
include(ECMMarkNonGuiExecutable)
include(KDEInstallDirs)
include(ECMInstallIcons)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings)
include(GenerateExportHeader)
include(FeatureSummary)

############## Find Packages ###################

find_package(Qt5 ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS
             Core
             Widgets
             Gui
)

find_package(KF5 REQUIRED COMPONENTS
             I18n
             Config
)

find_package(LibRaw ${LIBRAW_MIN_VERSION} REQUIRED)
set_package_properties("LibRaw"    PROPERTIES
                       DESCRIPTION "Required to build libkdcraw"
                       URL         "http://www.libraw.org"
                       TYPE        RECOMMENDED
                       PURPOSE     "Library to decode RAW image"
)

ecm_setup_version(${LIBKDCRAW_LIB_VERSION}
                  VARIABLE_PREFIX      KDCRAW
                  VERSION_HEADER       "src/libkdcraw_version.h"
                  PACKAGE_VERSION_FILE "KF5KDcrawConfigVersion.cmake"
                  SOVERSION            ${LIBKDCRAW_SO_VERSION})

############## Targets #########################

add_definitions(-DTRANSLATION_DOMAIN=\"libkdcraw\")

add_subdirectory(src)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif()

############## CMake Config Files ##############

message("${CMAKECONFIG_INSTALL_DIR}")
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5KDcraw")

ecm_configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/KF5KDcrawConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KF5KDcrawConfig.cmake"
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/KF5KDcrawConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/KF5KDcrawConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT   Devel
)

install(EXPORT      KF5KDcrawTargets
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE        KF5KDcrawTargets.cmake
        NAMESPACE   KF5::
)

# Install data
add_subdirectory(data)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
