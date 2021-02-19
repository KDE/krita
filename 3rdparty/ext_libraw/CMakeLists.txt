SET(PREFIX_ext_libraw "${EXTPREFIX}" )
ExternalProject_Add( ext_libraw
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://www.libraw.org/data/LibRaw-0.19.5.tar.gz
        URL_MD5 865ab9a40910709ff86988e8c0a7d146

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/LibRaw-cmake.diff
		COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/01-use_cmake_install_prefix.patch

        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_libraw} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

        UPDATE_COMMAND ""
    	DEPENDS ext_lcms2 ext_jpeg
)

