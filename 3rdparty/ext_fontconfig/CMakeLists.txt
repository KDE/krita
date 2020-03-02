SET(PREFIX_ext_fontconfig "${EXTPREFIX}" )
ExternalProject_Add( ext_fontconfig
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://www.freedesktop.org/software/fontconfig/release/fontconfig-2.13.1.tar.gz
    URL_MD5 690c6cb840a92fa8908cdf462d19ec66
  
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_fontconfig}  ${GLOBAL_AUTOMAKE_PROFILE}
    BUILD_COMMAND make
    INSTALL_COMMAND make install 

    UPDATE_COMMAND ""
)
