SET(PREFIX_ext_python "${EXTPREFIX}" )
if (UNIX)
    ExternalProject_Add( ext_python
	    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    	URL http://files.kde.org/krita/build/dependencies/Python-3.5.2.tar.gz
        URL_MD5 ea334d398990037a4b8be324bd475c83
        
        CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_python}  ${GLOBAL_AUTOMAKE_PROFILE}
        BUILD_COMMAND make
        INSTALL_COMMAND make install
                        COMMAND ${CMAKE_COMMAND} -E copy ${PREFIX_ext_python}/bin/python3 ${PREFIX_ext_python}/bin/python

        UPDATE_COMMAND ""
	    ALWAYS 0
    )
endif()
