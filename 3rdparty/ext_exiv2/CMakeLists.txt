SET(PREFIX_ext_exiv2  "${EXTPREFIX}" )
if (WIN32)
ExternalProject_Add( ext_exiv2
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/exiv2-0.26-trunk.tar.gz
    URL_MD5 5399e3b570d7f9205f0e76d47582da4c

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/tzname.patch
          COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/patch_mingw.patch
          COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/build_with_mingw8.diff

    INSTALL_DIR ${PREFIX_ext_exiv2}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_exiv2} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DEXIV2_ENABLE_BUILD_SAMPLES=OFF -DEXIV2_ENABLE_BUILD_PO=OFF -DEXIV2_ENABLE_NLS=OFF -DICONV_INCLUDE_DIR=${PREFIX_ext_exiv2}/include

    UPDATE_COMMAND ""
    DEPENDS ext_iconv ext_expat
)
else()
ExternalProject_Add( ext_exiv2
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/exiv2-0.26-trunk.tar.gz
    URL_MD5 5399e3b570d7f9205f0e76d47582da4c

    INSTALL_DIR ${PREFIX_ext_exiv2}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_exiv2} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DEXIV2_ENABLE_BUILD_SAMPLES=OFF -DEXIV2_ENABLE_BUILD_PO=OFF -DEXIV2_ENABLE_NLS=OFF -DICONV_INCLUDE_DIR=${PREFIX_ext_exiv2}/include

    UPDATE_COMMAND ""
    DEPENDS ext_iconv ext_expat
)
endif()
