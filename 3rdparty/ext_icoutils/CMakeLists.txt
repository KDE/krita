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

elseif (MSVC)

    ExternalProject_Add(
        ext_icoutils
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://files.kde.org/krita/build/dependencies/icoutils-bin-0.32.3.tar.gz
        URL_HASH SHA256=e2de7f65a82ff0f0377002f9cfb4182ec8bbd7de6d714bb7ae40af8da5d8a451

        INSTALL_DIR ${PREFIX_ext_icoutils}

        CONFIGURE_COMMAND ""
        BUILD_COMMAND ${CMAKE_COMMAND} -E echo Deploying icoutils
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/icotool.exe ${PREFIX_ext_icoutils}/bin/

        DEPENDS ext_png
        UPDATE_COMMAND ""
    )

endif (MINGW)
