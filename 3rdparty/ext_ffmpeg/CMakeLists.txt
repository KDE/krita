SET(PREFIX_ext_ffmpeg "${EXTPREFIX}")

ExternalProject_Add(
    ext_libogg
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://downloads.xiph.org/releases/ogg/libogg-1.3.5.tar.gz
    URL_HASH SHA256=0eb4b4b9420a0f51db142ba3f9c64b333f826532dc0f48c6410ae51f4799b664

    CMAKE_ARGS ${LIBOGG_EXTRA_CMAKE_ARGS} -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_ffmpeg} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}
)

ExternalProject_Add(
    ext_libvorbis
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://downloads.xiph.org/releases/vorbis/libvorbis-1.3.7.tar.xz
    URL_HASH SHA256=b33cc4934322bcbf6efcbacf49e3ca01aadbea4114ec9589d1b1e9d20f72954b

    CMAKE_ARGS ${LIBVORBIS_EXTRA_CMAKE_ARGS} -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_ffmpeg} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    DEPENDS ext_libogg
)

ExternalProject_Add(
  ext_ffmpeg
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  GIT_REPOSITORY https://anonscm.debian.org/git/pkg-multimedia/ffmpeg.git
  GIT_TAG debian/master
  CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_ffmpeg} --extra-ldflags="-L${PREFIX_ext_ffmpeg}/lib" --extra-cflags="-I${PREFIX_ext_ffmpeg}/include" --enable-shared --enable-avfilter --enable-gpl --disable-doc --enable-libvorbis
  BUILD_ALWAYS TRUE
  BUILD_IN_SOURCE 1
  BUILD_COMMAND make clean
  COMMAND make -j${SUBMAKE_JOBS}
  INSTALL_COMMAND make install

  DEPENDS ext_libvorbis
)
