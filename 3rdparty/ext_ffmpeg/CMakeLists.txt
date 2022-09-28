SET(PREFIX_ext_ffmpeg "${EXTPREFIX}")

ExternalProject_ADD(
  ext_ffmpeg
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  GIT_REPOSITORY https://anonscm.debian.org/git/pkg-multimedia/ffmpeg.git
  GIT_TAG debian/master
  CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_ffmpeg} --extra-ldflags="-L${PREFIX_ext_ffmpeg}/lib" --extra-cflags="-I${PREFIX_ext_ffmpeg}/include" --enable-shared --enable-avfilter --enable-gpl --disable-doc --enable-libvorbis
  BUILD_ALWAYS TRUE
  BUILD_IN_SOURCE 1
  BUILD_COMMAND $(MAKE) clean
  COMMAND $(MAKE)
  INSTALL_COMMAND $(MAKE) install
)

