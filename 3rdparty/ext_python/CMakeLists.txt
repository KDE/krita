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
elseif(MINGW)
    if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        ExternalProject_Add( ext_python
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://www.python.org/ftp/python/3.6.2/python-3.6.2-embed-amd64.zip
            URL_MD5 0fdfe9f79e0991815d6fc1712871c17f

            INSTALL_DIR ${PREFIX_ext_python}
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ${CMAKE_COMMAND} -E echo deploying python3 64-bit binary
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/ ${PREFIX_ext_python}/python
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/python3.dll ${PREFIX_ext_python}/bin
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/python36.dll ${PREFIX_ext_python}/bin
            UPDATE_COMMAND ""
        )
    else("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        ExternalProject_Add( ext_python
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://www.python.org/ftp/python/3.6.2/python-3.6.2-embed-win32.zip
            URL_MD5 2ca4768fdbadf6e670e97857bfab83e8

            INSTALL_DIR ${PREFIX_ext_python}
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ${CMAKE_COMMAND} -E echo deploying python3 32-bit binary
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/ ${PREFIX_ext_python}/python
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/python3.dll ${PREFIX_ext_python}/bin
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/python36.dll ${PREFIX_ext_python}/bin
            UPDATE_COMMAND ""
        )
    endif("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")

endif()
