SET(PREFIX_ext_zlib "${EXTPREFIX}" )
if (MSVC)
ExternalProject_Add(
    ext_zlib
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/zlib-1.2.7.tar.gz
    URL_MD5 60df6a37c56e7c1366cca812414f7b85

    INSTALL_DIR ${PREFIX_ext_zlib}
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.2.7-20130123.diff
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_zlib} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
    ALWAYS 0
    DEPENDS ext_patch
)
else ()
ExternalProject_Add( ext_zlib
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
	URL http://files.kde.org/krita/build/dependencies//zlib-1.2.7.tar.gz
	URL_MD5 60df6a37c56e7c1366cca812414f7b85
	
        INSTALL_DIR ${PREFIX_ext_zlib}
	PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.2.7.diff
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_zlib} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} 

	UPDATE_COMMAND ""
	ALWAYS 0
)
endif ()
