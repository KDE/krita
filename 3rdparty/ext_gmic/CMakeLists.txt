SET(PREFIX_ext_gmic "${EXTPREFIX}" )

# Download the gmic sources
ExternalProject_Add( ext_gmic_base
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://gmic.eu/files/source/gmic_2.7.1.tar.gz
    URL_MD5 11a947bb2fc1dfdd40995982c5c448eb

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/cmake.diff
          COMMAND  ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/no_threads.diff

    SOURCE_DIR gmic

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_gmic} -D -DCMAKE_BUILD_TYPE=Release ${GLOBAL_PROFILE}

    BUILD_COMMAND ""
    INSTALL_COMMAND ""

    BUILD_IN_SOURCE 1
)

# Download and build gmic-qt
# FIXME: Forcing CMAKE_BUILD_TYPE to Release
ExternalProject_Add( ext_gmic_qt
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/c-koi/gmic-qt/archive/v.2.7.0.tar.gz
    URL_MD5 b18577dfd5be0311bdad7737b56386dc
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/desktop_icon.diff
          COMMAND  ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/gmic-qt-cmake.diff

    SOURCE_DIR gmic-qt
    INSTALL_DIR ${PREFIX_ext_gmic}

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_gmic} -DGMIC_QT_HOST=krita -DCMAKE_BUILD_TYPE=Release ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""

    DEPENDS ext_gmic_base
)

add_custom_target(ext_gmic)
add_dependencies(ext_gmic ext_gmic_qt)
