SET(EXTPREFIX_fcitx-qt5 "${EXTPREFIX}" )

ExternalProject_Add(
    ext_fcitx-qt5
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/fcitx/fcitx-qt5/archive/refs/tags/1.2.5.tar.gz
    URL_MD5 5a2aa2ce28d766cdfcd7465699bae996

    SOURCE_DIR fcitx-qt5-1.2.5
    INSTALL_DIR ${EXTPREFIX_fcitx-qt5}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_fcitx-qt5} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DENABLE_LIBRARY=OFF

    DEPENDS ext_qt
    UPDATE_COMMAND ""
)
