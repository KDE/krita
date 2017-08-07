SET(PREFIX_ext_sip "${EXTPREFIX}" )
if (UNIX)
    ExternalProject_Add( ext_sip
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL http://files.kde.org/krita/build/dependencies/sip-4.18.tar.gz
        URL_MD5 78724bf2a79878201c3bc81a1d8248ea
        
        CONFIGURE_COMMAND ${PREFIX_ext_sip}/bin/python3 <SOURCE_DIR>/configure.py -b ${PREFIX_ext_sip}/bin -d ${PREFIX_ext_sip}/sip -e ${PREFIX_ext_sip}/include  --sipdir ${PREFIX_ext_sip}/sip --target-py-version 3.5
        BUILD_COMMAND make
        INSTALL_COMMAND make install
        
        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
    )
elseif (MINGW)
    list(APPEND _SIP_conf
        --platform win32-g++
        -b ${PREFIX_ext_sip}/bin
        -d ${PREFIX_ext_sip}/share/krita/pykrita
        -e ${PREFIX_ext_sip}/include
        --sipdir ${PREFIX_ext_sip}/share/sip
        --target-py-version 3.6
    )
    ExternalProject_Add( ext_sip
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://sourceforge.net/projects/pyqt/files/sip/sip-4.19.3/sip-4.19.3.zip
        URL_MD5 1098da9ee1915354fedf38fd6fbe22ce

        CONFIGURE_COMMAND python.exe <SOURCE_DIR>/configure.py ${_SIP_conf}
        BUILD_COMMAND mingw32-make -j${SUBMAKE_JOBS} LDFLAGS=${SECURITY_SHARED_LINKER_FLAGS}
        INSTALL_COMMAND mingw32-make -j${SUBMAKE_JOBS} install
        
        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
    )
endif()

