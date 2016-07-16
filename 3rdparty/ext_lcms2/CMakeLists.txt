SET(PREFIX_ext_lcms2 "${EXTPREFIX}" )
if (MSVC)
ExternalProject_Add( ext_lcms2
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
	URL http://files.kde.org/krita/build/dependencies/lcms2-2.8rc1.tar.gz
    URL_MD5 eb81dd3664ba25a69eb5d28f1c1915ba
		
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/lcms2-2.4-20130124.diff
	INSTALL_DIR ${PREFIX_ext_lcms2}
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_lcms2} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}
	
    UPDATE_COMMAND ""
	ALWAYS 0
	DEPENDS ext_patch
)
else (MSVC)
ExternalProject_Add( ext_lcms2
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
	URL http://files.kde.org/krita/build/dependencies/lcms2-2.8rc1.tar.gz
    URL_MD5 eb81dd3664ba25a69eb5d28f1c1915ba

    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_lcms2} ${GLOBAL_AUTOMAKE_PROFILE}
    BUILD_COMMAND make
    INSTALL_COMMAND make install

    UPDATE_COMMAND ""
    ALWAYS 0
)
endif (MSVC)
