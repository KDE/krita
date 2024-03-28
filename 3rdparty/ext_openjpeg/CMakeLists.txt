SET(PREFIX_ext_openjpeg "${EXTPREFIX}" )
ExternalProject_Add( ext_openjpeg
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    DOWNLOAD_NAME openjpeg-v2.5.0.tar.gz
    URL https://github.com/uclouvain/openjpeg/archive/v2.5.0.tar.gz
    URL_HASH SHA256=0333806d6adecc6f7a91243b2b839ff4d2053823634d4f6ed7a59bc87409122a

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Build-fix-linking-of-executables-on-some-systems-whe.patch

    INSTALL_DIR ${PREFIX_ext_openjpeg}
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_openjpeg} -DOPENJPEG_INSTALL_INCLUDE_DIR=include/ -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} -DBUILD_PKGCONFIG_FILES=ON ${GLOBAL_PROFILE}
	
    UPDATE_COMMAND ""
    DEPENDS ext_lcms2
)
