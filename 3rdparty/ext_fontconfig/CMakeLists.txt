SET(PREFIX_ext_fontconfig "${EXTPREFIX}" )
ExternalProject_Add( ext_fontconfig
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/fontconfig-2.12.0.tar.gz
    URL_MD5 d8b056231abcb6257db6dc6d745360b2
  
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_fontconfig}  ${GLOBAL_AUTOMAKE_PROFILE}
    BUILD_COMMAND make
    INSTALL_COMMAND make install      

    UPDATE_COMMAND ""
)
