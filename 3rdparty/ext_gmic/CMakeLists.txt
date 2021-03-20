SET(PREFIX_ext_gmic "${EXTPREFIX}" )

# Download the gmic sources
ExternalProject_Add( ext_gmic_base
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://gmic.eu/files/source/gmic_2.9.6.tar.gz
    URL_HASH SHA256=d22fe8515af16dcd050d09a0bc7127ec29f4795ce732a0c6c3eaf839356bc11a

    SOURCE_DIR gmic

    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""

    BUILD_IN_SOURCE 1
)

# Download and build gmic-qt
ExternalProject_Add( ext_gmic_qt
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/c-koi/gmic-qt/archive/v.2.9.6.tar.gz
    URL_HASH SHA256=40712fb8b2e8d39a5a2c6349bc14277444db25c81b1cf51fde1c159ef622942b
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Add-desktop-entry.patch

    SOURCE_DIR gmic-qt
    INSTALL_DIR ${PREFIX_ext_gmic}

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_gmic} -DGMIC_QT_HOST=krita -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""

    DEPENDS ext_gmic_base ext_fftw3
)

add_custom_target(ext_gmic)
add_dependencies(ext_gmic ext_gmic_qt)
