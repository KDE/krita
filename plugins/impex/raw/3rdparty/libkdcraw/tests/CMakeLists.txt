#
# Copyright (c) 2010-2014, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

add_subdirectory(multithreading)

set(raw2png_SRCS raw2png.cpp)
add_executable(raw2png ${raw2png_SRCS})
target_link_libraries(raw2png KF5KDcraw)

set(libinfo_SRCS libinfo.cpp)
add_executable(libinfo ${libinfo_SRCS})
target_link_libraries(libinfo KF5KDcraw)
