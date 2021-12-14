SET(PREFIX_ext_seexpr "${EXTPREFIX}" )
ExternalProject_Add(
    ext_seexpr
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.kde.org/stable/kseexpr/4.0.4/kseexpr-4.0.4.0.tar.gz
    URL_MD5 207a7f20fe8995cb0ae90424495a1c67

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_seexpr} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} -DFLAVOR=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DENABLE_LLVM_BACKEND=OFF -DBUILD_DOC=OFF -DBUILD_TESTS=OFF -DBUILD_UTILS=OFF

    UPDATE_COMMAND ""
    DEPENDS ext_qt ext_extra_cmake_modules ext_ki18n
)
