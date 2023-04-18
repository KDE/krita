SET(EXTPREFIX_fribidi "${EXTPREFIX}" )

if(APPLE)
ExternalProject_Add(
    ext_fribidi
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/fribidi/fribidi/releases/download/v1.0.11/fribidi-1.0.11.tar.xz
    URL_HASH SHA256=30f93e9c63ee627d1a2cedcf59ac34d45bf30240982f99e44c6e015466b4e73d

	INSTALL_DIR ${EXTPREFIX_fribidi}
	CONFIGURE_COMMAND ./configure --prefix=${EXTPREFIX_fribidi} ${GLOBAL_AUTOMAKE_PROFILE}

    UPDATE_COMMAND ""
	BUILD_IN_SOURCE 1
)

else()
    ExternalProject_Add(
        ext_fribidi
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/fribidi/fribidi/releases/download/v1.0.11/fribidi-1.0.11.tar.xz
        URL_HASH SHA256=30f93e9c63ee627d1a2cedcf59ac34d45bf30240982f99e44c6e015466b4e73d

        CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env
            PYTHONPATH=${_krita_pythonpath}
            PKG_CONFIG_ALLOW_SYSTEM_CFLAGS=1
            ${MESON_BINARY_PATH} setup <BINARY_DIR> <SOURCE_DIR>
                --prefix=${EXTPREFIX_fribidi}
                --libdir=lib
                -Dbuildtype=$<IF:$<CONFIG:Debug>,debug,debugoptimized>
                ${EXTRA_MESON_FLAGS}

        BUILD_COMMAND ${CMAKE_COMMAND} -E env
            PYTHONPATH=${_krita_pythonpath}
            ${MESON_BINARY_PATH} compile -C <BINARY_DIR> -j${SUBMAKE_JOBS}

        INSTALL_COMMAND ${CMAKE_COMMAND} -E env
            PYTHONPATH=${_krita_pythonpath}
            ${MESON_BINARY_PATH} install -C <BINARY_DIR>

        DEPENDS ${MESON_DEP} ${PKG_CONFIG_DEP}
    )

endif()