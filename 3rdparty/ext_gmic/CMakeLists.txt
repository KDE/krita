SET(PREFIX_ext_gmic "${EXTPREFIX}" )

# Download the gmic sources
if (WIN32)
    # gmic uses a shell script with wget to download some files which won't work
    # on Windows, so we use CMake to download them instead
    set(_GMIC_BUILD_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/gmic-downloads.cmake)
else (WIN32)
    set(_GMIC_BUILD_COMMAND make -C src CImg.h gmic_stdlib.h)
endif (WIN32)
ExternalProject_Add( ext_gmic_base
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    GIT_REPOSITORY https://github.com/dtschump/gmic.git
    GIT_TAG master
    GIT_SHALLOW 1

    SOURCE_DIR gmic

    CONFIGURE_COMMAND ""
    BUILD_COMMAND ${_GMIC_BUILD_COMMAND}
    INSTALL_COMMAND ""

    BUILD_IN_SOURCE 1
)

# Download and build gmic-qt
# FIXME: Forcing CMAKE_BUILD_TYPE to Release
ExternalProject_Add( ext_gmic_qt
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/c-koi/gmic-qt/archive/v.211.tar.gz
    URL_HASH SHA1=776db5f77a4044711a9c4a4b0c6533556f4b810d

    SOURCE_DIR gmic-qt
    INSTALL_DIR ${PREFIX_ext_gmic}

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_gmic} -DGMIC_QT_HOST=krita -DCMAKE_BUILD_TYPE=Release ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy <BINARY_DIR>/gmic_krita_qt${CMAKE_EXECUTABLE_SUFFIX} <INSTALL_DIR>/bin/gmic_krita_qt${CMAKE_EXECUTABLE_SUFFIX}

    DEPENDS ext_gmic_base
)

add_custom_target(ext_gmic)
add_dependencies(ext_gmic ext_gmic_qt)
