#
# Copyright (c) 2010-2015, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

kde_enable_exceptions()

include_directories(${LibRaw_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/..)

add_definitions(${LibRaw_DEFINITIONS})

remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII)

set(kdcraw_LIB_SRCS kdcraw.cpp
                    kdcraw_p.cpp
                    rsliderspinbox.cpp
                    rnuminput.cpp
                    rexpanderbox.cpp
                    rcombobox.cpp
                    rwidgetutils.cpp
                    dcrawsettingswidget.cpp
                    dcrawinfocontainer.cpp
                    rawdecodingsettings.cpp
                    squeezedcombobox.cpp
                    ractionthreadbase.cpp
                    ractionjob.cpp
                    libkdcraw_debug.cpp
)

ecm_generate_headers(kdcraw_CamelCase_HEADERS

                     HEADER_NAMES
                     KDcraw
                     DcrawSettingsWidget
                     DcrawInfoContainer
                     RawDecodingSettings
                     RComboBox
                     RNumInput
                     RExpanderBox
                     SqueezedComboBox
                     RActionThreadBase
                     RActionJob
                     RawFiles
                     RWidgetUtils

                     PREFIX           KDCRAW
                     REQUIRED_HEADERS kdcraw_HEADERS
)

add_library(KF5KDcraw SHARED ${kdcraw_LIB_SRCS})
add_library(KF5::KDcraw ALIAS KF5KDcraw)

generate_export_header(KF5KDcraw BASE_NAME libkdcraw EXPORT_MACRO_NAME )

set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE OFF)

target_include_directories(KF5KDcraw INTERFACE
                           "$<INSTALL_INTERFACE:${KF5_INCLUDE_INSTALL_DIR}/KDCRAW;${KF5_INCLUDE_INSTALL_DIR}/KDCRAW/kdcraw>"
                           "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
)

target_link_libraries(KF5KDcraw
                      PRIVATE
                      ${LibRaw_LIBRARIES}
                      PUBLIC
                      Qt5::Core
                      Qt5::Widgets
                      Qt5::Gui
                      KF5::I18n
                      KF5::ConfigCore
)

set_target_properties(KF5KDcraw   PROPERTIES
                      VERSION     ${KDCRAW_VERSION_STRING}
                      SOVERSION   ${KDCRAW_SOVERSION}
                      EXPORT_NAME KDcraw
)

install(TARGETS KF5KDcraw
        EXPORT  KF5KDcrawTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS}
)

install(FILES       ${kdcraw_CamelCase_HEADERS}
        DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KDCRAW/KDCRAW
        COMPONENT   Devel
)

install(FILES       ${kdcraw_HEADERS}
        DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KDCRAW/kdcraw
        COMPONENT   Devel
)

install(FILES       ${CMAKE_CURRENT_BINARY_DIR}/libkdcraw_export.h
        DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KDCRAW/kdcraw
        COMPONENT   Devel
)

install(FILES       "${CMAKE_CURRENT_BINARY_DIR}/libkdcraw_version.h"
        DESTINATION "${KF5_INCLUDE_INSTALL_DIR}"
        COMPONENT   Devel
)
