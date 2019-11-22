SET(PREFIX_ext_gettext "${EXTPREFIX}" )
if (MINGW)
    if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        ExternalProject_Add(ext_gettext_bin
        
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://github.com/mlocati/gettext-iconv-windows/releases/download/v0.20.1-v1.16/gettext0.20.1-iconv1.16-static-64.zip
            URL_MD5 cff2994c78d421d8a3afdc9231ed8aa9

            BUILD_COMMAND ${CMAKE_COMMAND} -E echo Deploying gettext 64-bit dependencies
            CONFIGURE_COMMAND ""
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR> ${PREFIX_ext_gettext}
                    
            UPDATE_COMMAND ""
        )
    else("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
            ExternalProject_Add(ext_gettext_bin
        
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://github.com/mlocati/gettext-iconv-windows/releases/download/v0.20.1-v1.16/gettext0.20.1-iconv1.16-static-32.zip
            URL_MD5 ba11885580103ab45dfe6337a3fe56e5

            BUILD_COMMAND ${CMAKE_COMMAND} -E echo Deploying gettext 64-bit dependencies
            CONFIGURE_COMMAND ""
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR> ${PREFIX_ext_gettext}

            UPDATE_COMMAND ""
    )
    endif()
    ExternalProject_Add (
        ext_gettext_lib
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://ftp.gnu.org/gnu/gettext/gettext-0.18.tar.gz
        URL_MD5 d52a3e061032a1ed13856d42fc86f0fd
        PATCH_COMMAND ${PATCH_COMMAND} --binary -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/gettext-0.18-20130319.diff
        INSTALL_DIR ${PREFIX_ext_gettext}
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_gettext} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}
        UPDATE_COMMAND ""
        DEPENDS ext_patch ext_iconv
    )
    add_custom_target(ext_gettext)
    add_dependencies(ext_gettext ext_gettext_bin ext_gettext_lib)
else (MINGW)
ExternalProject_Add( ext_gettext
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
   URL https://ftp.gnu.org/pub/gnu/gettext/gettext-0.20.tar.gz
   URL_MD5 e71133e1bad4f2ce83121078fd33edde
    
    INSTALL_DIR ${PREFIX_ext_gettext}
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_gettext} --disable-java   ${GLOBAL_AUTOMAKE_PROFILE} --disable-native-java 
   BUILD_COMMAND make
   INSTALL_COMMAND make install

    UPDATE_COMMAND ""
    DEPENDS ext_iconv
)
endif (MINGW)
