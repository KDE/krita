SET(PREFIX_ext_seexpr "${EXTPREFIX}" )
ExternalProject_Add(
    ext_seexpr
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.kde.org/stable/kseexpr/4.0.3/kseexpr-4.0.3.0.tar.gz
    URL_MD5 724965d273c92b6373d5e1e0ae6f7eb1

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Do-not-look-LLVM-up-unless-explicitly-enabled.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0002-Reject-malformed-curve-and-ccurve-calls-at-parse-tim.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0003-ExprEditor-replace-index-for-line-column-when-report.patch

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_seexpr} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} -DFLAVOR=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DENABLE_LLVM_BACKEND=OFF -DBUILD_DOC=OFF -DBUILD_TESTS=OFF -DBUILD_UTILS=OFF

    UPDATE_COMMAND ""
    DEPENDS ext_qt ext_extra_cmake_modules ext_ki18n
)
