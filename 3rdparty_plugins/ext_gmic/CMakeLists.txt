SET(PREFIX_ext_gmic "${EXTPREFIX}" )

ExternalProject_Add( ext_gmic
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://files.kde.org/krita/build/dependencies/gmic-3.0.0.1-patched.tar.gz
    URL_HASH SHA256=05e25367af01039a97c52d48694f00daf36ee180fa68220f72522aaf245a782b

    SOURCE_SUBDIR gmic-qt

    INSTALL_DIR ${PREFIX_ext_gmic}

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_gmic} -DGMIC_QT_HOST=krita-plugin -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}
    LIST_SEPARATOR "|"

    UPDATE_COMMAND ""
)
