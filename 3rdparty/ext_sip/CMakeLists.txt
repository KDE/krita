SET(PREFIX_ext_sip "${EXTPREFIX}" )
if (UNIX)
  SET(PYTHON_EXECUTABLE_PATH ${PREFIX_ext_sip}/bin/python3)
  if(NOT EXISTS ${PYTHON_EXECUTABLE_PATH})
    message("WARNING: using system python3!")
    SET(PYTHON_EXECUTABLE_PATH python3)
  endif()

  ExternalProject_Add( ext_sip
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://www.riverbankcomputing.com/static/Downloads/sip/sip-4.19.14.tar.gz
        URL_MD5 bba62b2ea7e16193c7dfcfadb8dd0d05

        CONFIGURE_COMMAND ${PYTHON_EXECUTABLE_PATH} <SOURCE_DIR>/configure.py -b ${PREFIX_ext_sip}/bin -d ${PREFIX_ext_sip}/sip -e ${PREFIX_ext_sip}/include  --sipdir ${PREFIX_ext_sip}/sip --target-py-version 3.5
        BUILD_COMMAND make
        INSTALL_COMMAND make install

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
    )
elseif (MINGW)
    list(APPEND _SIP_conf
        --platform win32-g++
        -b ${PREFIX_ext_sip}/bin
        -d ${PREFIX_ext_sip}/lib/krita-python-libs
        -e ${PREFIX_ext_sip}/include
        --sipdir ${PREFIX_ext_sip}/share/sip
        --target-py-version 3.6
    )
    ExternalProject_Add( ext_sip
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://www.riverbankcomputing.com/static/Downloads/sip/sip-4.19.14.tar.gz
        URL_MD5 bba62b2ea7e16193c7dfcfadb8dd0d05

        CONFIGURE_COMMAND ${PYTHON_EXECUTABLE} <SOURCE_DIR>/configure.py ${_SIP_conf}
        BUILD_COMMAND mingw32-make -j${SUBMAKE_JOBS} LDFLAGS=${SECURITY_SHARED_LINKER_FLAGS}
        INSTALL_COMMAND mingw32-make -j${SUBMAKE_JOBS} install
        
        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
    )
endif()

