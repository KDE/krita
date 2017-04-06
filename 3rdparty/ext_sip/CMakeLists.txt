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
        ALWAYS 0
    )
elseif (MINGW)
    ExternalProject_Add( ext_sip
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL http://files.kde.org/krita/build/dependencies/sip-4.19.2.zip
        URL_MD5 c7a45d67dc9018d2211a395b3eae1bd1
        
        CONFIGURE_COMMAND python.exe <SOURCE_DIR>/configure.py --platform win32-g++ -b ${PREFIX_ext_sip}/bin -d ${PREFIX_ext_sip}/sip -e ${PREFIX_ext_sip}/include  --sipdir ${PREFIX_ext_sip}/sip --target-py-version 3.6
        BUILD_COMMAND make
        INSTALL_COMMAND make install
        
        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
        ALWAYS 0
    )
endif()

