SET(EXTPREFIX_raqm "${EXTPREFIX}" ) 

if(NOT MESON_BINARY_PATH)
    set(MESON_BINARY_PATH ${EXTPREFIX_raqm}/bin/meson)
endif()

if(NOT PKG_CONFIG_EXECUTABLE)
    set(PKG_CONFIG_EXECUTABLE ${EXTPREFIX_raqm}/bin/pkgconf)
endif()

ExternalProject_Add(
    ext_raqm
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/therahedwig/libraqm/archive/793d7a5e89f3a2bdda8d9236d24294e48dc2e354.zip
    URL_HASH SHA256=576e63009a26d613aab8dd3bd216be43a49979d26aa99cb186a0008a9f3af252
    
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/fix_utf_16.patch

    CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env
        CC="${CMAKE_C_COMPILER}"
        CXX="${CMAKE_CXX_COMPILER}"
        PYTHONPATH=${_krita_pythonpath}
        PKG_CONFIG="${PKG_CONFIG_EXECUTABLE}"
        ${MESON_BINARY_PATH} setup <BINARY_DIR> <SOURCE_DIR>
            --prefix=${EXTPREFIX_raqm}
            --libdir=lib
            --default-library=shared
            -Dbuildtype=$<IF:$<CONFIG:Debug>,debug,debugoptimized>
            -Dc_args=${_security_c_flags}
            -Dcpp_args=${_security_cxx_flags}
            -Dc_link_args=${_security_exe_linker_flags}
            -Dcpp_link_args=${_security_exe_linker_flags}
            -Dpkg_config_path=${EXTPREFIX_raqm}/share/pkgconfig,${EXTPREFIX_raqm}/lib/pkgconfig
            ${EXTRA_MESON_FLAGS}

    BUILD_COMMAND ${CMAKE_COMMAND} -E env
        PYTHONPATH=${_krita_pythonpath}
        ${MESON_BINARY_PATH} compile -C <BINARY_DIR> -j${SUBMAKE_JOBS}

    INSTALL_COMMAND ${CMAKE_COMMAND} -E env
        PYTHONPATH=${_krita_pythonpath}
        ${MESON_BINARY_PATH} install -C <BINARY_DIR>

    DEPENDS ext_freetype ext_harfbuzz ext_fribidi ${MESON_DEP} ${PKG_CONFIG_DEP}
)
