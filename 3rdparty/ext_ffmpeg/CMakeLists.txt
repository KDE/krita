SET(PREFIX_ext_ffmpeg "${EXTPREFIX}" )
ExternalProject_Add( ext_ffmpeg
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/ffmpeg-2.8.7.tar.xz
    URL_MD5 7e5b1c98eedfc3a364fa8c4095deeae5
  
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_ffmpeg}  ${GLOBAL_AUTOMAKE_PROFILE} -disable-debug \
        --enable-memalign-hack \
        --disable-pthreads \
        --enable-w32threads \
        --disable-doc \
        --enable-avresample \
        --enable-gpl \
        --disable-programs

    BUILD_COMMAND make
    INSTALL_COMMAND make install      

    UPDATE_COMMAND ""
    ALWAYS 0
)
