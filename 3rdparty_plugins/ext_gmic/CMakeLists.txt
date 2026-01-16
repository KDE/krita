SET(PREFIX_ext_gmic "${EXTPREFIX}" )

ExternalProject_Add( ext_gmic
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/vanyossi/gmic/releases/download/v3.6.6.2/gmic-3.6.6.2.tar.gz
    URL_HASH SHA256=3ccec8c0cf848b3fdf5a843fe66b793e28c34a2a4f8f9c9a2ce96a5d0840b650

    SOURCE_SUBDIR gmic-qt

    INSTALL_DIR ${PREFIX_ext_gmic}

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_gmic} -DGMIC_QT_HOST=krita-plugin -DENABLE_SYSTEM_GMIC=FALSE -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}
    LIST_SEPARATOR "|"

    UPDATE_COMMAND ""
)
