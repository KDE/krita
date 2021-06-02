SET(PREFIX_ext_icoutils "${EXTPREFIX}")

if (MINGW)

    ExternalProject_Add(
        ext_icoutils
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL http://savannah.nongnu.org/download/icoutils/icoutils-0.32.3.tar.bz2
        URL_HASH SHA256=17abe02d043a253b68b47e3af69c9fc755b895db68fdc8811786125df564c6e0

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Really-do-not-include-libintl.h.patch
              COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0002-CMake-to-build-icotool.exe.patch
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_icoutils} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

        DEPENDS ext_png
        UPDATE_COMMAND ""
    )

endif (MINGW)
