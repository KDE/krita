#
# SPDX-License-Identifier: BSD-3-Clause
#
# KDDockWidgets_FOUND               - KDDockWidgets library was found
# KDDockWidgets_INCLUDE_DIR         - Path to KDDockWidgets include dir
# KDDockWidgets_LIBRARIES           - List of KDDockWidgets libraries

find_package(KDDockWidgets-Qt5 QUIET)
IF (KDDockWidgets-Qt5_FOUND)
        set(KDDockWidgets_INCLUDE_DIRS KDDockWidgets::KDDockWidgets)
        set(KDDockWidgets_LIBRARIES KDDockWidgets::KDDockWidgets)
        set(KDDockWidgets_FOUND TRUE)
ELSEIF (KDDockWidgets_INCLUDE_DIRS AND KDDockWidgets_LIBRARIES)
	# in cache already
	SET(KDDockWidgets_FOUND TRUE)
ELSE ()
    IF (Qt5Core_FOUND)
        set(KDDockWidgets_LIB_VERSION_SUFFIX 5)
    ENDIF()
	IF (WIN32)
		FIND_PATH(KDDockWidgets_LIBRARY_DIR
			WIN32_DEBUG_POSTFIX d
            NAMES libKDDockWidgets${KDDockWidgets_LIB_VERSION_SUFFIX}.dll
			HINTS "C:/Programme/" "C:/Program Files"
			PATH_SUFFIXES KDDockWidgets/lib
		)
        FIND_LIBRARY(KDDockWidgets_LIBRARIES NAMES libkddockwidgets${KDDockWidgets_LIB_VERSION_SUFFIX}.dll HINTS ${KDDockWidgets_LIBRARY_DIR})
		FIND_PATH(KDDockWidgets_INCLUDE_DIR NAMES kddockwidgets_version.h HINTS ${KDDockWidgets_LIBRARY_DIR}/../ PATH_SUFFIXES include/kddockwidgets${KDDockWidgets_LIB_VERSION_SUFFIX})
	ELSE(WIN32)
		FIND_PACKAGE(PkgConfig)
		pkg_check_modules(PC_kddockwidgets kddockwidgets)
		FIND_LIBRARY(KDDockWidgets_LIBRARIES
			WIN32_DEBUG_POSTFIX d
            NAMES kddockwidgets${KDDockWidgets_LIB_VERSION_SUFFIX}
			HINTS /usr/lib /usr/lib64
		)
		FIND_PATH(KDDockWidgets_INCLUDE_DIR kddockwidgets_version.h
			HINTS /usr/include /usr/local/include
			PATH_SUFFIXES kddockwidgets${KDDockWidgets_LIB_VERSION_SUFFIX}
		)
	ENDIF (WIN32)
	INCLUDE(FindPackageHandleStandardArgs)
	SET(KDDockWidgets_INCLUDE_DIRS ${KDDockWidgets_INCLUDE_DIR})
	find_package_handle_standard_args(KDDockWidgets DEFAULT_MSG  KDDockWidgets_LIBRARIES KDDockWidgets_INCLUDE_DIR KDDockWidgets_INCLUDE_DIRS)
ENDIF ()
