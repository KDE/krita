SET(EXTPREFIX_qt  "${EXTPREFIX}")
if (WIN32)
ExternalProject_Add(
    ext_qt
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://download.qt.io/official_releases/qt/5.6/5.6.0/single/qt-everywhere-opensource-src-5.6.0.zip
    URL_MD5 03402708f260dcc917abae9bc559c1df
    PATCH_COMMAND  ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/disable-wintab.diff
          COMMAND  ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/qtgui-private-headers.diff

    INSTALL_DIR ${EXTPREFIX_qt}
    CONFIGURE_COMMAND <SOURCE_DIR>/configure.bat -skip qt3d -skip qtactiveqt -skip qtcanvas3d -skip qtconnectivity -skip qtdoc -skip qtenginio -skip qtgraphicaleffects -skip qtlocation -skip qtmultimedia -skip qtsensors -skip qtserialport -skip qtwayland -skip qtwebchannel -skip qtwebengine -skip qtwebsockets -skip qtwebview -skip qtxmlpatterns -no-sql-sqlite -nomake examples -nomake tools -no-compile-examples -no-dbus -no-iconv -no-angle -no-ssl -no-openssl -no-wmf-backend -no-qml-debug -no-libproxy -no-system-proxies -no-nis -no-icu -no-mtdev -opensource -confirm-license -release -opengl desktop -qt-zlib -qt-pcre -qt-libpng -qt-libjpeg -prefix ${EXTPREFIX_qt}

    BUILD_COMMAND nmake
    INSTALL_COMMAND nmake install
    UPDATE_COMMAND ""
    BUILD_IN_SOURCE 1
    ALWAYS 0
)
else() 
    if (APPLE)
        ExternalProject_Add(
           ext_qt
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL http://download.qt.io/official_releases/qt/5.6/5.6.0/single/qt-everywhere-opensource-src-5.6.0.tar.gz
        URL_MD5 7a2a867bc12384f4161809136d49d4be
        INSTALL_DIR ${EXTPREFIX_qt}
        CONFIGURE_COMMAND "<SOURCE_DIR>/configure -confirm-license  -opensource -nomake examples \
                -no-openssl \
                -no-compile-examples -qt-freetype -qt-harfbuzz -opengl desktop \
                -qt-zlib -qt-pcre -qt-libpng -qt-libjpeg 
                -skip qt3d -skip qtactiveqt -skip qtcanvas3d -skip qtconnectivity \
                -skip qtenginio -skip qtgraphicaleffects -skip qtlocation -skip qtmultimedia \
                -skip qtwayland -skip qtwebchannel -skip qtwebengine -skip qtwebsockets \
                -skip qtwebview \
                -prefix ${EXTPREFIX_qt} "
        BUILD_COMMAND make
        INSTALL_COMMAND make install
        UPDATE_COMMAND ""
        BUILD_IN_SOURCE 1
        ALWAYS 0
    )
    else()
        ExternalProject_Add(
            ext_qt
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL http://download.qt.io/official_releases/qt/5.6/5.6.0/single/qt-everywhere-opensource-src-5.6.0.tar.gz
            URL_MD5 7a2a867bc12384f4161809136d49d4be
            PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/qt-no-motion-compression.diff
            INSTALL_DIR ${EXTPREFIX_qt}
            CONFIGURE_COMMAND <SOURCE_DIR>/configure -prefix ${EXTPREFIX_qt} -opensource -confirm-license -nomake examples -no-sql-sqlite -no-openssl -no-qml-debug -no-mtdev  -no-journald  -no-syslog -no-nis  -no-cups -no-tslib -no-directfb -no-linuxfb -no-libproxy -no-pch -qt-zlib -qt-pcre -qt-libpng -qt-libjpeg  -qt-harfbuzz -qt-freetype -qt-xcb -qt-xkbcommon-x11 -optimized-qmake -skip qt3d -skip qtactiveqt -skip qtcanvas3d -skip qtconnectivity -skip qtenginio -skip qtgraphicaleffects -skip qtlocation -skip qtmultimedia -skip qtwayland -skip qtwebchannel -skip qtwebengine -skip qtwebsockets -skip qtwebview 
            
            BUILD_COMMAND make
            INSTALL_COMMAND make install
            UPDATE_COMMAND ""
            BUILD_IN_SOURCE 1
            ALWAYS 0
        )
    endif()
endif()
