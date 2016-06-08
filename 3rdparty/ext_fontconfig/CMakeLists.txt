SET(PREFIX_ext_fontconfig "${EXTPREFIX}" )
ExternalProject_Add( ext_fontconfig
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/fontconfig-2.11.94.tar.gz
    URL_MD5 479be870c7f83f15f87bac085b61d641
  
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_fontconfig}  ${GLOBAL_AUTOMAKE_PROFILE}
    BUILD_COMMAND make
    INSTALL_COMMAND make install      

    UPDATE_COMMAND ""
    ALWAYS 0
)
