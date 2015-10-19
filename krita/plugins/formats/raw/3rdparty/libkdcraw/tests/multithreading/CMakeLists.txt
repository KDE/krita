#
# Copyright (c) 2010-2014, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include_directories(${KF5_INCLUDE_DIR}
                    ${Qt5_INCLUDE_DIR}
                    ${Qt5_QTCORE_INCLUDE_DIR}
                    ${LibRaw_INCLUDE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

set(multicorerawtopng_SRCS
    main.cpp
    processordlg.cpp
    actionthread.cpp
)

add_executable(multicorerawtopng ${multicorerawtopng_SRCS})

target_link_libraries(multicorerawtopng
                      Qt5::Gui
                      Qt5::Core
                      KF5KDcraw
)
