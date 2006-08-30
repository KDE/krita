# - Try to find QCA2 (Qt Cryptography Architecture 2)
# Once done this will define
#
#  QCA2_FOUND - system has QCA2
#  QCA2_INCLUDE_DIR - the QCA2 include directory
#  QCA2_LIBRARIES - the libraries needed to use QCA2
#  QCA2_DEFINITIONS - Compiler switches required for using QCA2
#
# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls


INCLUDE(UsePkgConfig)

PKGCONFIG(qca _Qca2IncDir _Qca2LinkDir _Qca2LinkFlags _Qca2Cflags)

MESSAGE(STATUS "pkg-config returned ${_Qca2IncDir} for QCA 2 includes")
set(QCA2_DEFINITIONS ${_Qca2Cflags})

set(QCA2_INCLUDE_DIR ${_Qca2IncDir})

#FIND_PATH(QCA2_INCLUDE_DIR QtCrypto
#  PATHS
# ${_Qca2IncDir}
#)

MESSAGE(STATUS "QCA2_INCLUDE_DIR has value ${QCA2_INCLUDE_DIR}" )

FIND_LIBRARY(QCA2_LIBRARIES NAMES qca
  PATHS
  ${_Qca2LinkDir} 
  NO_DEFAULT_PATH 
)

if (QCA2_INCLUDE_DIR AND QCA2_LIBRARIES)
   set(QCA2_FOUND TRUE)
endif (QCA2_INCLUDE_DIR AND QCA2_LIBRARIES)

if (QCA2_FOUND)
  if (NOT QCA2_FIND_QUIETLY)
    message(STATUS "Found QCA2: ${QCA2_LIBRARIES}")
  endif (NOT QCA2_FIND_QUIETLY)
else (QCA2_FOUND)
  if (QCA2_FIND_REQUIRED)
    message(SEND_ERROR "Could NOT find QCA2")
  endif (QCA2_FIND_REQUIRED)
endif (QCA2_FOUND)

MARK_AS_ADVANCED(QCA2_INCLUDE_DIR QCA2_LIBRARIES)
