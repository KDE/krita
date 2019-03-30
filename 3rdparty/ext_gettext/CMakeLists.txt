SET(PREFIX_ext_gettext "${EXTPREFIX}" )
if (MINGW)
    if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        ExternalProject_Add(ext_gettext
        
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL http://files.kde.org/krita/build/dependencies/gettext0.19.8.1-iconv1.14-shared-64.zip
            URL_MD5 4c6b70667a83e6828749b93c283a4832

            BUILD_COMMAND ${CMAKE_COMMAND} -E echo Deploying gettext 64-bit dependencies
            CONFIGURE_COMMAND ""
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR> ${PREFIX_ext_gettext}
                    
            UPDATE_COMMAND ""
        )
    else("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
            ExternalProject_Add(ext_gettext
        
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL http://files.kde.org/krita/build/dependencies/gettext0.19.8.1-iconv1.14-shared-32.zip
            URL_MD5 42f0db42b37a89ef2ebb64186fe1ee32

            BUILD_COMMAND ${CMAKE_COMMAND} -E echo Deploying gettext 64-bit dependencies
            CONFIGURE_COMMAND ""
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR> ${PREFIX_ext_gettext}

            UPDATE_COMMAND ""
    )
    endif()
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
