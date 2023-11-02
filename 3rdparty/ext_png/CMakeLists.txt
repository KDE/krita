SET(PREFIX_ext_png "${EXTPREFIX}" )

kis_ExternalProject_Add_with_separate_builds_apple(
    ext_png
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://downloads.sourceforge.net/project/libpng/libpng16/1.6.40/libpng-1.6.40.tar.xz
    URL_HASH SHA256=535b479b2467ff231a3ec6d92a525906fb8ef27978be4f66dbe05d3f3a01b3a1

	INSTALL_DIR ${PREFIX_ext_png}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_png} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DPNG_TESTS=OFF

	UPDATE_COMMAND ""

    DEPENDS ${ZLIB_DEP} 
)
