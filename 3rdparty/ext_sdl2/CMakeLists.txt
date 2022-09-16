SET(EXT_PREFIX_sdl2 "${EXTPREFIX}" )

ExternalProject_Add(
  ext_sdl2
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL https://libsdl.org/release/SDL2-2.0.9.tar.gz
  URL_MD5 f2ecfba915c54f7200f504d8b48a5dfe

  CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${EXT_PREFIX_sdl2}
  BUILD_COMMAND $(MAKE)
  INSTALL_COMMAND $(MAKE) install
)
