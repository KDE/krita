SET(PREFIX_ext_gmic "${EXTPREFIX}" )

ExternalProject_Add( ext_gmic
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://files.kde.org/krita/build/dependencies/gmic-3.0.2.1-patched.tar.gz
    URL_HASH SHA256=7c09144583ab071b0bcedbd4292ee22ba5b3a73e92d44f7551f1ee03c3fea05d

    SOURCE_SUBDIR gmic-qt

    INSTALL_DIR ${PREFIX_ext_gmic}

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_gmic} -DGMIC_QT_HOST=krita-plugin -DENABLE_SYSTEM_GMIC=FALSE -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}
    LIST_SEPARATOR "|"

    UPDATE_COMMAND ""
)
