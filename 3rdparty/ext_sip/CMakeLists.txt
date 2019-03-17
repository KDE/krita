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
        URL https://www.riverbankcomputing.com/static/Downloads/sip/4.19.15/sip-4.19.15.tar.gz
        URL_MD5 236578d2199da630ae1251671b9a7bfe

        CONFIGURE_COMMAND ${PYTHON_EXECUTABLE_PATH} <SOURCE_DIR>/configure.py -b ${PREFIX_ext_sip}/bin -d ${PREFIX_ext_sip}/sip -e ${PREFIX_ext_sip}/include  --sipdir ${PREFIX_ext_sip}/sip --target-py-version 3.5 --sip-module PyQt5.sip
        BUILD_COMMAND make
        INSTALL_COMMAND make install

            BUILD_IN_SOURCE 1

            UPDATE_COMMAND ""
        )
    else(APPLE)
        find_package(PythonInterp 3.5)
        if(NOT PYTHONINTERP_FOUND)
            SET(PYTHON_EXECUTABLE ${PREFIX_ext_sip}/bin/python3)
            if(NOT EXISTS "${PYTHON_EXECUTABLE}")
                message("WARNING: using system python3!")
                SET(PYTHON_EXECUTABLE python3)
            endif()
            # Get PYTHONVERSION
            execute_process(COMMAND ${PYTHON_EXECUTABLE} -V COMMAND awk "{print $2}" OUTPUT_VARIABLE PYTHON_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
            string(REPLACE "." ";" PYTHON_VERSION_LIST ${PYTHON_VERSION})
            list(GET PYTHON_VERSION_LIST 0 PYTHON_VERSION_MAJOR)
            list(GET PYTHON_VERSION_LIST 1 PYTHON_VERSION_MINOR)
        endif()

        set(PY_VER "${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}")

        ExternalProject_Add( ext_sip
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://www.riverbankcomputing.com/static/Downloads/sip/sip-4.19.14.tar.gz
            URL_MD5 bba62b2ea7e16193c7dfcfadb8dd0d05

            CONFIGURE_COMMAND ${PYTHON_EXECUTABLE} <SOURCE_DIR>/configure.py -b ${PREFIX_ext_sip}/bin -d ${PREFIX_ext_sip}/sip -e ${PREFIX_ext_sip}/include --sipdir ${PREFIX_ext_sip}/sip --target-py-version ${PY_VER} --sip-module PyQt5.sip
            BUILD_COMMAND make
            INSTALL_COMMAND make install
                COMMAND ${CMAKE_COMMAND} -E copy ${PREFIX_ext_sip}/sip/PyQt5/sip.so ${PREFIX_ext_sip}/lib/python${PY_VER}/site-packages/PyQt5/sip.so
                COMMAND ${CMAKE_COMMAND} -E copy ${PREFIX_ext_sip}/sip/PyQt5/sip.pyi ${PREFIX_ext_sip}/lib/python${PY_VER}/site-packages/PyQt5/sip.pyi

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
        --target-py-version 3.6
        --sip-module PyQt5.sip
    )
    ExternalProject_Add( ext_sip
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://www.riverbankcomputing.com/static/Downloads/sip/4.19.15/sip-4.19.15.zip
        URL_MD5 4a1a4760cfabef15d68f45a6920974c2

        CONFIGURE_COMMAND ${PYTHON_EXECUTABLE} <SOURCE_DIR>/configure.py ${_SIP_conf}
        BUILD_COMMAND mingw32-make -j${SUBMAKE_JOBS} LDFLAGS=${SECURITY_SHARED_LINKER_FLAGS}
        INSTALL_COMMAND mingw32-make -j${SUBMAKE_JOBS} install
        
        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
    )
endif()

