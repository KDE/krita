SET(PREFIX_ext_gmic "${EXTPREFIX}" )

ExternalProject_Add( ext_gmic
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://files.kde.org/krita/build/dependencies/gmic-3.1.2.1-patched.tar.gz
    URL_HASH SHA256=5781cb88fbad4353123e0e1df227f3db5da9ea1ec6700035c5efe3f4e429208b

    SOURCE_SUBDIR gmic-qt

    INSTALL_DIR ${PREFIX_ext_gmic}

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_gmic} -DGMIC_QT_HOST=krita-plugin -DENABLE_SYSTEM_GMIC=FALSE -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}
    LIST_SEPARATOR "|"

    UPDATE_COMMAND ""
)
