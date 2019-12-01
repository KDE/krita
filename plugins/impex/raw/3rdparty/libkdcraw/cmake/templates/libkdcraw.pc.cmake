prefix=${CMAKE_INSTALL_PREFIX}
exec_prefix=${BIN_INSTALL_DIR}
libdir=${LIB_INSTALL_DIR}
includedir=${INCLUDE_INSTALL_DIR}

Name: ${PROJECT_NAME}
Description: A C++ wrapper around LibRaw library to decode RAW pictures. This library is used by digiKam and kipi-plugins.
URL: https://commits.kde.org/libkdcraw
Requires:
Version: ${DCRAW_LIB_VERSION_STRING}
Libs: -L${LIB_INSTALL_DIR} -lkdcraw
Cflags: -I${INCLUDE_INSTALL_DIR}
