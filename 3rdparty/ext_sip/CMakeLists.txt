SET(PREFIX_ext_sip "${EXTPREFIX}" )
if (UNIX)
    if(NOT APPLE)
        SET(PYTHON_EXECUTABLE_PATH ${PREFIX_ext_sip}/bin/python3)
        if(NOT EXISTS ${PYTHON_EXECUTABLE_PATH})
            message("WARNING: using system python3!")
            SET(PYTHON_EXECUTABLE_PATH python3)
        endif()

        ExternalProject_Add( ext_sip
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://pypi.python.org/packages/source/s/sip/sip-6.1.0.tar.gz
            URL_HASH SHA256=f069d550dd819609e019e5dc58fc5193e081c7f3fb4f7dc8f9be734e34d4e56e

            CONFIGURE_COMMAND ""

            BUILD_COMMAND ${PYTHON_EXECUTABLE_PATH} <SOURCE_DIR>/setup.py build -j ${SUBMAKE_JOBS}

            INSTALL_COMMAND ${PYTHON_EXECUTABLE_PATH} <SOURCE_DIR>/setup.py install --skip-build --prefix=${PREFIX_ext_sip} --optimize=1

            BUILD_IN_SOURCE 1

            UPDATE_COMMAND ""
        )
    else(APPLE)
        if(NOT PYTHONINTERP_FOUND)
            SET(PYTHON_EXECUTABLE ${PREFIX_ext_sip}/bin/python3)
            if(NOT EXISTS "${PYTHON_EXECUTABLE}")
                message("WARNING: using system python3!")
                SET(PYTHON_EXECUTABLE python3)
            endif()
        endif()

        ExternalProject_Add( ext_sip
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://files.kde.org/krita/build/dependencies/sip-4.19.19.tar.gz
            URL_MD5 98111479309dc472410f26080d6d4a88

            CMAKE_ARGS -DPYTHON_INCLUDE_DIR=${PREFIX_ext_sip}/lib/Python.framework/Headers
            CONFIGURE_COMMAND ${PYTHON_EXECUTABLE} <SOURCE_DIR>/configure.py --deployment-target=10.11
                -b ${PREFIX_ext_sip}/bin -d ${PREFIX_ext_sip}/lib/Python.framework/Versions/Current/site-packages/ 
                -e ${PREFIX_ext_sip}/include --sipdir ${PREFIX_ext_sip}/share/sip --target-py-version 3.8 --sip-module PyQt5.sip
            BUILD_COMMAND make

            # force single-threaded installation to avoid
            # race conditions in post-install .py script
            INSTALL_COMMAND make -j1 install
                # COMMAND ${CMAKE_COMMAND} -E create_symlink ./PyQt5/sip.so ${PREFIX_ext_sip}/lib/Python.framework/Versions/Current/site-packages/sip.so
                # COMMAND ${CMAKE_COMMAND} -E create_symlink ./PyQt5/sip.pyi ${PREFIX_ext_sip}/lib/Python.framework/Versions/Current/site-packages/sip.pyi

            BUILD_IN_SOURCE 1

            UPDATE_COMMAND ""
        )
    endif()
elseif (MINGW)
    list(APPEND _SIP_conf
        --platform win32-g++
        -b ${PREFIX_ext_sip}/bin
        -d ${PREFIX_ext_sip}/lib/krita-python-libs
        -e ${PREFIX_ext_sip}/include
        --sipdir ${PREFIX_ext_sip}/share/sip
        --target-py-version 3.8
        --sip-module PyQt5.sip
    )
    ExternalProject_Add( ext_sip
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://files.kde.org/krita/build/dependencies/sip-4.19.19.tar.gz
        URL_MD5 98111479309dc472410f26080d6d4a88

        CONFIGURE_COMMAND ${PYTHON_EXECUTABLE} <SOURCE_DIR>/configure.py ${_SIP_conf}
        BUILD_COMMAND mingw32-make -j${SUBMAKE_JOBS} LDFLAGS=${SECURITY_SHARED_LINKER_FLAGS}

        # force single-threaded installation to avoid
        # race conditions in post-install .py script
        INSTALL_COMMAND mingw32-make -j1 install
        
        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
    )
endif()

