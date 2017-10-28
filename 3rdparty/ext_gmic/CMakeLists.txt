SET(PREFIX_ext_gmic "${EXTPREFIX}" )

# Download the gmic sources
ExternalProject_Add( ext_gmic_base
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://gmic.eu/files/source/gmic_2.1.3.tar.gz
    URL_HASH SHA1=f0832766f009a74287bb9fdbe013f4a84db1ad76

    SOURCE_DIR gmic

    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""

    BUILD_IN_SOURCE 1
)

# Download and build gmic-qt
# FIXME: Forcing CMAKE_BUILD_TYPE to Release
ExternalProject_Add( ext_gmic_qt
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/c-koi/gmic-qt/archive/v.213.tar.gz
    DOWNLOAD_NAME gmic-qt_2.1.3.tar.gz
    URL_HASH SHA1=8c6489da2ce44ae3eae6ab7e9cf7b45716c0aacd

    SOURCE_DIR gmic-qt
    INSTALL_DIR ${PREFIX_ext_gmic}

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_gmic} -DGMIC_QT_HOST=krita -DCMAKE_BUILD_TYPE=Release ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy <BINARY_DIR>/gmic_krita_qt${CMAKE_EXECUTABLE_SUFFIX} <INSTALL_DIR>/bin/gmic_krita_qt${CMAKE_EXECUTABLE_SUFFIX}

    DEPENDS ext_gmic_base
)

add_custom_target(ext_gmic)
add_dependencies(ext_gmic ext_gmic_qt)
