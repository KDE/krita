SET(PREFIX_ext_gettext "${EXTPREFIX}" )
if (MSVC OR MINGW)
    if(CMAKE_CL_64)
        ExternalProject_Add( ext_gettext
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL http://files.kde.org/krita/build/dependencies/gettext0.19.8.1-iconv1.14-shared-64.zip
            URL_MD5 f877ebff42736535415b4ddb9c631b86
            CONFIGURE_COMMAND ""
        	BUILD_COMMAND ${CMAKE_COMMAND} -E echo deploying gettext.exe binary
	            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/bin ${PREFIX_ext_gettext}/bin
                    COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/lib ${PREFIX_ext_gettext}/lib
                    COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/include ${PREFIX_ext_gettext}/include

        UPDATE_COMMAND ""
        ALWAYS 0
        )
    else()
        ExternalProject_Add( ext_gettext
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL http://files.kde.org/krita/build/dependencies/gettext0.19.8.1-iconv1.14-shared-32.zip
            URL_MD5 bced59a375aef8d325b746f0dfbe8f48
            CONFIGURE_COMMAND ""
    	    BUILD_COMMAND ${CMAKE_COMMAND} -E echo deploying gettext.exe binary
	                INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/bin ${PREFIX_ext_gettext}/bin
                        COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/lib ${PREFIX_ext_gettext}/lib
                        COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/include ${PREFIX_ext_gettext}/include
            UPDATE_COMMAND ""
            ALWAYS 0
        )
    endif()
else (MSVC OR MINGW)
    ExternalProject_Add( ext_gettext
	    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL http://files.kde.org/krita/build/dependencies/gettext-0.19.8.tar.gz
        URL_MD5 e4fffc004f21596becd1055cf36be31d
	
    	INSTALL_DIR ${PREFIX_ext_gettext}
	    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_gettext} --disable-java   ${GLOBAL_AUTOMAKE_PROFILE} --disable-native-java 
        BUILD_COMMAND make
        INSTALL_COMMAND make install

    	UPDATE_COMMAND ""
	    ALWAYS 0
    	DEPENDS ext_iconv
    )
endif (MSVC OR MINGW)
