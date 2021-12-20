SET(PREFIX_ext_gettext "${EXTPREFIX}" )
if (WIN32)
    if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        ExternalProject_Add(ext_gettext_bin
        
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://github.com/mlocati/gettext-iconv-windows/releases/download/v0.21-v1.16/gettext0.21-iconv1.16-static-64.zip
            URL_HASH SHA256=721395c2e057eeed321f0c793311732e57cb4fa30d5708672a13902a69a77d43

            BUILD_COMMAND ${CMAKE_COMMAND} -E echo Deploying gettext 64-bit dependencies
            CONFIGURE_COMMAND ""
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR> ${PREFIX_ext_gettext}
                    
            UPDATE_COMMAND ""
        )
    else("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
            ExternalProject_Add(ext_gettext_bin
        
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://github.com/mlocati/gettext-iconv-windows/releases/download/v0.21-v1.16/gettext0.21-iconv1.16-static-32.zip
            URL_HASH SHA256=73a1b64a15fff9e6a5dc980d0a69cf3a24bde51bd47091a1b3fef68827ef23a8

            BUILD_COMMAND ${CMAKE_COMMAND} -E echo Deploying gettext 32-bit dependencies
            CONFIGURE_COMMAND ""
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR> ${PREFIX_ext_gettext}

            UPDATE_COMMAND ""
    )
    endif()
    ExternalProject_Add (
        ext_gettext_lib
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://files.kde.org/krita/build/dependencies/gettext-0.21.tar.xz
        URL_HASH MD5SUM=40996bbaf7d1356d3c22e33a8b255b31

        PATCH_COMMAND ${PATCH_COMMAND} --binary -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Fix-macro-definitions.patch
            COMMAND ${PATCH_COMMAND} --binary -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0002-Fix-uwp-build.patch
            COMMAND ${PATCH_COMMAND} --binary -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0003-Fix-win-unicode-paths.patch
            COMMAND ${PATCH_COMMAND} --binary -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0004-vcpkg-CMake-toolchain.patch
        INSTALL_DIR ${PREFIX_ext_gettext}
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_gettext} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}
        UPDATE_COMMAND ""
        DEPENDS ext_patch DEPENDS ${ICONV_DEP}
    )
    add_custom_target(ext_gettext)
    add_dependencies(ext_gettext ext_gettext_bin ext_gettext_lib)
else (WIN32)
ExternalProject_Add( ext_gettext
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://files.kde.org/krita/build/dependencies/gettext-0.21.tar.xz
    URL_HASH MD5SUM=40996bbaf7d1356d3c22e33a8b255b31
    PATCH_COMMAND ${PATCH_COMMAND} -Z -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Fix-libgettextpo-linking-on-macOS.patch

    INSTALL_DIR ${PREFIX_ext_gettext}
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_gettext} --disable-java   ${GLOBAL_AUTOMAKE_PROFILE} --disable-native-java 
   BUILD_COMMAND make
   INSTALL_COMMAND make install

    UPDATE_COMMAND ""
    DEPENDS ext_iconv
)
endif (WIN32)
