SET(PREFIX_ext_python "${EXTPREFIX}" )
if (UNIX)
    ExternalProject_Add( ext_python
	    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    	URL http://files.kde.org/krita/build/dependencies/Python-3.5.2.tar.gz
        URL_MD5 ea334d398990037a4b8be324bd475c83
        
        CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_python}  ${GLOBAL_AUTOMAKE_PROFILE} --enable-shared
        BUILD_COMMAND make
        INSTALL_COMMAND make install
                        COMMAND ${CMAKE_COMMAND} -E copy ${PREFIX_ext_python}/bin/python3 ${PREFIX_ext_python}/bin/python

        UPDATE_COMMAND ""
	    ALWAYS 0
    )
else()
    if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        ExternalProject_Add( ext_python3
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL http://files.kde.org/krita/build/dependencies/python-3.6.1-embed-amd64.zip
            URL_MD5 708496ebbe9a730d19d5d288afd216f1

            INSTALL_DIR ${PREFIX_ext_python3}
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ${CMAKE_COMMAND} -E echo deploying python3 64-bit binary
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/ ${PREFIX_ext_python3}/bin
            UPDATE_COMMAND ""
        )
    else("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        ExternalProject_Add( ext_python3
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL http://files.kde.org/krita/build/dependencies/python-3.6.1-embed-win32.zip
            URL_MD5 8dff09a1b19b7a7dcb915765328484cf

            INSTALL_DIR ${PREFIX_ext_python3}
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ${CMAKE_COMMAND} -E echo deploying python3 32-bit binary
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/ ${PREFIX_ext_python3}/bin


            UPDATE_COMMAND ""
        )
    endif("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")

endif()
