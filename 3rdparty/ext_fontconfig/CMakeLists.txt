SET(PREFIX_ext_fontconfig "${EXTPREFIX}" )
ExternalProject_Add( ext_fontconfig
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://www.freedesktop.org/software/fontconfig/release/fontconfig-2.13.94.tar.gz
    URL_MD5 6098ee9e444c21fab75992b2633e876e
  
    # patch from: https://gitlab.freedesktop.org/fontconfig/fontconfig/-/commit/889097353ecd7b061ae7cf677e3db56db77a135f
    PATCH_COMMAND ${PATCH_COMMAND} -Z --binary -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Fix-FC_DEFAULT_FONTS-on-macOS.patch
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_fontconfig}  ${GLOBAL_AUTOMAKE_PROFILE}

    UPDATE_COMMAND ""
)

if(APPLE)
ExternalProject_Add_Step( ext_fontconfig fix_rpath
        COMMENT "Fixing <BINARY_DIR>/fc-cache/.libs/fc-cache rpath"
        COMMAND install_name_tool -add_rpath ${PREFIX_ext_fontconfig}/lib <BINARY_DIR>/fc-cache/.libs/fc-cache
        DEPENDEES build
    )
endif()
