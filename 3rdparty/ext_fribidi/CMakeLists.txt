SET(EXTPREFIX_fribidi "${EXTPREFIX}" )

if(NOT MESON_BINARY_PATH)
    set(MESON_BINARY_PATH ${EXTPREFIX_fribidi}/bin/meson)
endif()

if(NOT PKG_CONFIG_EXECUTABLE)
    set(PKG_CONFIG_EXECUTABLE ${EXTPREFIX_fribidi}/bin/pkgconf)
endif()

ExternalProject_Add(
    ext_fribidi
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/fribidi/fribidi/releases/download/v1.0.11/fribidi-1.0.11.tar.xz
    URL_HASH SHA256=30f93e9c63ee627d1a2cedcf59ac34d45bf30240982f99e44c6e015466b4e73d

    CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env
        CC="${CMAKE_C_COMPILER}"
        CXX="${CMAKE_CXX_COMPILER}"
        PYTHONPATH=${_krita_pythonpath}
        PKG_CONFIG=${PKG_CONFIG_EXECUTABLE}
        ${MESON_BINARY_PATH} setup <BINARY_DIR> <SOURCE_DIR>
            --prefix=${EXTPREFIX_fribidi}
            --libdir=lib
            -Dbuildtype=$<IF:$<CONFIG:Debug>,debug,debugoptimized>
            -Dc_args=${_security_c_flags}
            -Dcpp_args=${_security_cxx_flags}
            -Dc_link_args=${_security_exe_linker_flags}
            -Dcpp_link_args=${_security_exe_linker_flags}
            -Dpkg_config_path=${EXTPREFIX_fribidi}/share/pkgconfig,${EXTPREFIX_fribidi}/lib/pkgconfig

    BUILD_COMMAND ${CMAKE_COMMAND} -E env
        PYTHONPATH=${_krita_pythonpath}
        ${MESON_BINARY_PATH} compile -C <BINARY_DIR> -j${SUBMAKE_JOBS}

    INSTALL_COMMAND ${CMAKE_COMMAND} -E env
        PYTHONPATH=${_krita_pythonpath}
        ${MESON_BINARY_PATH} install -C <BINARY_DIR>

    DEPENDS ${MESON_DEP} ${PKG_CONFIG_DEP}
)
