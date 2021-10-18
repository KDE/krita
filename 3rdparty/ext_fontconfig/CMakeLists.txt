SET(PREFIX_ext_fontconfig "${EXTPREFIX}" )
ExternalProject_Add( ext_fontconfig
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://www.freedesktop.org/software/fontconfig/release/fontconfig-2.13.94.tar.gz
    URL_MD5 6098ee9e444c21fab75992b2633e876e
  
    # patch from: https://gitlab.freedesktop.org/fontconfig/fontconfig/-/commit/889097353ecd7b061ae7cf677e3db56db77a135f
    PATCH_COMMAND ${PATCH_COMMAND} --binary -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/macOS_sedErrorFix.diff
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_fontconfig}  ${GLOBAL_AUTOMAKE_PROFILE}
    BUILD_COMMAND make
    INSTALL_COMMAND make install 

    UPDATE_COMMAND ""
)
