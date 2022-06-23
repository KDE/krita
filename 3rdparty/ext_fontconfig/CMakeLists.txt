SET(PREFIX_ext_fontconfig "${EXTPREFIX}" )
ExternalProject_Add( ext_fontconfig
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://www.freedesktop.org/software/fontconfig/release/fontconfig-2.14.0.tar.gz
    URL_HASH SHA256=b8f607d556e8257da2f3616b4d704be30fd73bd71e367355ca78963f9a7f0434

    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_fontconfig}  ${GLOBAL_AUTOMAKE_PROFILE}

    UPDATE_COMMAND ""
)
