SET(PREFIX_ext_python "${EXTPREFIX}" )
if (UNIX)
    if (APPLE)
        ExternalProject_Add( ext_python
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://www.python.org/ftp/python/3.10.7/Python-3.10.7.tar.xz
            URL_MD5 b8094f007b3a835ca3be6bdf8116cccc

            PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/osx_fixappinstall.diff

            # Setuptools needs OpenSSL 1.1, the stock libssl isn't enough
            CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env
                PATH="${PREFIX_ext_python}/lib/Python.framework/Versions/Current/bin:$ENV{PATH}"
                <SOURCE_DIR>/configure MACOSX_DEPLOYMENT_TARGET=10.13 --prefix=${PREFIX_ext_python} ${GLOBAL_AUTOMAKE_PROFILE} --with-openssl=${PREFIX_ext_python}
                --enable-framework=${PREFIX_ext_python}/lib --enable-optimizations --with-suffix=no

            # force single-threaded installation to avoid race conditions
            INSTALL_COMMAND ${CMAKE_COMMAND} -E env
                PATH="${PREFIX_ext_python}/lib/Python.framework/Versions/Current/bin:$ENV{PATH}"
                make -j1 install
                COMMAND ${CMAKE_COMMAND} -E copy ${PREFIX_ext_python}/bin/python3 ${PREFIX_ext_python}/bin/python
                COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/sitecustomize.py ${PREFIX_ext_python}/lib/Python.framework/Versions/Current/lib/python${PYTHON_VERSION}/
                COMMAND ${CMAKE_COMMAND} -E create_symlink ${PREFIX_ext_python}/lib/Python.framework/Versions/Current/lib/python${PYTHON_VERSION} ${PREFIX_ext_python}/lib/python${PYTHON_VERSION}
                COMMAND ${CMAKE_COMMAND} -E create_symlink ${PREFIX_ext_python}/lib/Python.framework/Versions/Current/include/python${PYTHON_VERSION} ${PREFIX_ext_python}/include/python${PYTHON_VERSION}
                COMMAND ${CMAKE_COMMAND} -E create_symlink ./lib/python${PYTHON_VERSION}/site-packages ${PREFIX_ext_python}/lib/Python.framework/Versions/Current/site-packages
                # CMake FindPythonLib can't find framework libraries, lack of maintainer for Python
                COMMAND find ${PREFIX_ext_python}/lib/Python.framework/Versions/Current/lib -type l -d 1
                | grep -o "[^/]*$"
                | xargs -I FILE ${CMAKE_COMMAND} -E create_symlink ./Python.framework/Python ${PREFIX_ext_python}/lib/FILE

            UPDATE_COMMAND ""

            DEPENDS ext_openssl
        )

        ExternalProject_Add_Step( ext_python update_sip
            COMMENT "Updating pip"
            COMMAND COMMAND ${PREFIX_ext_python}/bin/python3 -m pip install --upgrade pip
            DEPENDEES install
        )

        if (APPLE)
            ExternalProject_Add_Step( ext_python update_certifi
                COMMENT "Deploying certifi to our OpenSSL CApath"
                COMMAND ${PREFIX_ext_python}/bin/python3 ${CMAKE_CURRENT_SOURCE_DIR}/install_certificates.py
                DEPENDEES update_sip
            )
        endif()
    else()

    ExternalProject_Add( ext_python
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://www.python.org/ftp/python/3.10.7/Python-3.10.7.tar.xz
        URL_MD5 b8094f007b3a835ca3be6bdf8116cccc

        CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_python} ${GLOBAL_AUTOMAKE_PROFILE} --enable-shared

        UPDATE_COMMAND ""
    )

    ExternalProject_Add_Step( ext_python post_install
        COMMAND ${CMAKE_COMMAND} -E copy ${PREFIX_ext_python}/bin/python3 ${PREFIX_ext_python}/bin/python
        DEPENDEES install
    )
    endif()
elseif(WIN32)
        ExternalProject_Add( ext_python
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://www.python.org/ftp/python/3.8.1/python-3.8.1-embed-amd64.zip
        URL_MD5 4d091857a2153d9406bb5c522b211061

        INSTALL_DIR ${PREFIX_ext_python}
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ${CMAKE_COMMAND} -E echo deploying python3 64-bit binary
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/ ${PREFIX_ext_python}/python
                COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/python3.dll ${PREFIX_ext_python}/bin
                COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/python38.dll ${PREFIX_ext_python}/bin
                COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/vcruntime140.dll ${PREFIX_ext_python}/bin
                COMMAND ${CMAKE_COMMAND} -E make_directory ${PREFIX_ext_python}/lib/site-packages/
                COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/sitecustomize.py ${PREFIX_ext_python}/lib/site-packages/
        UPDATE_COMMAND ""
    )
endif()
