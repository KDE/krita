SET(PREFIX_ext_seexpr "${EXTPREFIX}" )
ExternalProject_Add(
    ext_seexpr
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://files.kde.org/krita/build/dependencies/seexpr-3.4.4.0-alpha.1.tar.gz
    URL_MD5 35b415bcbd278ccd5ef5220b8b0595fe

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Use-application-specific-data-directory-for-translat.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0002-Remove-static-library-support.patch

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_seexpr} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} -DFLAVOR=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DBOOST_INCLUDE_DIR=${PREFIX_ext_seexpr}/include -DBOOST_ROOT=${PREFIX_ext_seexpr} -DBOOST_LIB_DIR=${PREFIX_ext_seexpr}/lib -DENABLE_LLVM_BACKEND=OFF -DENABLE_SSE4=OFF -DBUILD_DOC=OFF -DBUILD_TESTS=OFF -DBUILD_UTILS=OFF -DUSE_PYTHON=OFF

    UPDATE_COMMAND ""
    DEPENDS ext_qt ext_extra_cmake_modules
)
