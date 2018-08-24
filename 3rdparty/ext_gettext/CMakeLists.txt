SET(PREFIX_ext_gettext "${EXTPREFIX}" )
if (MINGW)
ExternalProject_Add(
    ext_gettext
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/gettext-0.18.tar.gz
    URL_MD5 d52a3e061032a1ed13856d42fc86f0fd

    PATCH_COMMAND ${PATCH_COMMAND} --binary -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/gettext-0.18-20130319.diff
    INSTALL_DIR ${PREFIX_ext_gettext}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_gettext} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
    DEPENDS ext_patch ext_iconv
)
else (MINGW)
ExternalProject_Add( ext_gettext
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/gettext-0.19.8.tar.gz
    URL_MD5 e4fffc004f21596becd1055cf36be31d
	
	INSTALL_DIR ${PREFIX_ext_gettext}
	CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_gettext} --disable-java   ${GLOBAL_AUTOMAKE_PROFILE} --disable-native-java 
        BUILD_COMMAND make
        INSTALL_COMMAND make install

	UPDATE_COMMAND ""
	DEPENDS ext_iconv
)
endif (MINGW)
