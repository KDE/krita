SET(EXTPREFIX_unibreak "${EXTPREFIX}" ) 

ExternalProject_Add(
    ext_unibreak
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/adah1972/libunibreak/releases/download/libunibreak_5_0/libunibreak-5.0.tar.gz
    URL_HASH SHA256=58f2fe4f9d9fc8277eb324075ba603479fa847a99a4b134ccb305ca42adf7158

    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${EXTPREFIX_unibreak} ${GLOBAL_AUTOMAKE_PROFILE}
    BUILD_COMMAND make
    INSTALL_COMMAND make install

    UPDATE_COMMAND ""
) 
