SET(PREFIX_ext_python "${EXTPREFIX}" )
if (UNIX)
    if (APPLE)
    set(PYTHON_VERSION "3.8")
    ExternalProject_Add( ext_python
	    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://www.python.org/ftp/python/3.8.1/Python-3.8.1.tar.xz
        URL_MD5 b3fb85fd479c0bf950c626ef80cacb57
        
        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/osx_fixappinstall.diff

        CONFIGURE_COMMAND <SOURCE_DIR>/configure MACOSX_DEPLOYMENT_TARGET=10.13 --prefix=${PREFIX_ext_python} ${GLOBAL_AUTOMAKE_PROFILE}
            --without-ensurepip --enable-framework=${PREFIX_ext_python}/lib --enable-optimizations
        BUILD_COMMAND make
        INSTALL_COMMAND make install
            COMMAND ${CMAKE_COMMAND} -E copy ${PREFIX_ext_python}/bin/python3 ${PREFIX_ext_python}/bin/python
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/sitecustomize.py ${PREFIX_ext_python}/lib/Python.framework/Versions/Current/lib/python${PYTHON_VERSION}/
            COMMAND ${CMAKE_COMMAND} -E create_symlink ./lib/python${PYTHON_VERSION}/site-packages ${PREFIX_ext_python}/lib/Python.framework/Versions/Current/site-packages
            # CMake FindPythonLib can't find framework libraries, lack of maintainer for Python
            COMMAND find ${PREFIX_ext_python}/lib/Python.framework/Versions/Current/lib -type l -d 1
            | grep -o "[^/]*$"
            | xargs -I FILE ${CMAKE_COMMAND} -E create_symlink ./Python.framework/Python ${PREFIX_ext_python}/lib/FILE

        UPDATE_COMMAND ""
	    ALWAYS 0
    )
    else()

    ExternalProject_Add( ext_python
	    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://www.python.org/ftp/python/3.8.1/Python-3.8.1.tar.xz
        URL_MD5 b3fb85fd479c0bf950c626ef80cacb57
        
        CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_python} ${GLOBAL_AUTOMAKE_PROFILE} --enable-shared
        BUILD_COMMAND make
        INSTALL_COMMAND make install
                        COMMAND ${CMAKE_COMMAND} -E copy ${PREFIX_ext_python}/bin/python3 ${PREFIX_ext_python}/bin/python

        UPDATE_COMMAND ""
	    ALWAYS 0
    )
    endif()
elseif(MINGW)
    if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        ExternalProject_Add( ext_python
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://www.python.org/ftp/python/3.8.0/python-3.8.0-embed-amd64.zip
            URL_MD5 99cca948512b53fb165084787143ef19

            INSTALL_DIR ${PREFIX_ext_python}
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ${CMAKE_COMMAND} -E echo deploying python3 64-bit binary
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/ ${PREFIX_ext_python}/python
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/python3.dll ${PREFIX_ext_python}/bin
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/python38.dll ${PREFIX_ext_python}/bin
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/vcruntime140.dll ${PREFIX_ext_python}/bin
            UPDATE_COMMAND ""
        )
    else("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        ExternalProject_Add( ext_python
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://www.python.org/ftp/python/3.8.0/python-3.8.0-embed-win32.zip
            URL_MD5 2ec3abf05f3f1046e0dbd1ca5c74ce88

            INSTALL_DIR ${PREFIX_ext_python}
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ${CMAKE_COMMAND} -E echo deploying python3 32-bit binary
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/ ${PREFIX_ext_python}/python
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/python3.dll ${PREFIX_ext_python}/bin
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/python38.dll ${PREFIX_ext_python}/bin
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/vcruntime140.dll ${PREFIX_ext_python}/bin
            UPDATE_COMMAND ""
        )
    endif("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")

endif()
