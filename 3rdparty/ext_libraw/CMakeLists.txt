SET(PREFIX_ext_libraw "${EXTPREFIX}" )

if(APPLE)
    # There is no OPENMP support on apple's clang
    set(LIBRAW_CMAKE_FLAGS "-DENABLE_OPENMP=OFF")
endif()

ExternalProject_Add( ext_libraw
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://www.libraw.org/data/LibRaw-0.20.2.tar.gz
        URL_HASH SHA256=dc1b486c2003435733043e4e05273477326e51c3ea554c6864a4eafaff1004a6

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-LibRaw-cmake.patch
            COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0002-Use-CMake-install-prefix.patch

        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_libraw} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${LIBRAW_CMAKE_FLAGS} ${GLOBAL_PROFILE}

        UPDATE_COMMAND ""
    	DEPENDS ext_lcms2 ext_jpeg
)

