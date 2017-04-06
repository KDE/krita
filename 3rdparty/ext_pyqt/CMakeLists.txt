SET(PREFIX_ext_pyqt "${EXTPREFIX}" )
if (UNIX)
    ExternalProject_Add( ext_pyqt
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL http://files.kde.org/krita/build/dependencies/PyQt5_gpl-5.6.tar.gz
        URL_MD5 dbfc885c0548e024ba5260c4f44e0481
        
        CONFIGURE_COMMAND ${PREFIX_ext_pyqt}/bin/python3 <SOURCE_DIR>/configure.py --confirm-license
        BUILD_COMMAND make
        INSTALL_COMMAND make install
        
        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
        ALWAYS 0
    )
endif()

