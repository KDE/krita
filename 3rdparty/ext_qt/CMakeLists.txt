SET(EXTPREFIX_qt  "${EXTPREFIX}")
if (WIN32)
ExternalProject_Add(
    ext_qt
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://download.qt.io/development_releases/qt/5.6/5.6.0-beta/single/qt-everywhere-opensource-src-5.6.0-beta.zip
    URL_MD5 d0c366ddf9796256e168ccce6b087175
    PATCH_COMMAND myptch -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/disable-wintab.diff

    INSTALL_DIR ${EXTPREFIX_qt}
    CONFIGURE_COMMAND <SOURCE_DIR>/configure.bat -no-sql-sqlite -opensource -nomake examples -confirm-license -no-dbus  -prefix ${EXTPREFIX_qt} -I ${EXTPREFIX_qt}/include -L ${EXTPREFIX_qt}/lib -nomake tools -no-compile-examples -opengl desktop -no-iconv -no-angle -no-ssl -no-openssl -no-wmf-backend -no-qml-debug -skip qt3d -skip qtactiveqt -skip qtcanvas3d -skip qtconnectivity -skip qtdoc -skip qtenginio -skip qtgraphicaleffects -skip qtlocation -skip qtmultimedia-skip qtsensors -skip qtserialport -skip qtwayland -skip qtwebchannel -skip qtwebengine -skip qtwebsockets -skip qtwebview -skip qtxmlpatterns -qt-zlib -qt-pcre -no-icu -qt-libpng -qt-libjpeg -no-pch

    BUILD_COMMAND nmake
    INSTALL_COMMAND nmake install
    UPDATE_COMMAND ""
    BUILD_IN_SOURCE 1
    ALWAYS 0
    #DEPENDS  ext_perl 
)
else() 
    if (APPLE)
        ExternalProject_Add(
           ext_qt
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
           URL http://download.qt.io/development_releases/qt/5.6/5.6.0-beta/single/qt-everywhere-opensource-src-5.6.0-beta.tar.gz
        URL_MD5 d0c366ddf9796256e168ccce6b087175
        INSTALL_DIR ${EXTPREFIX_qt}
        CONFIGURE_COMMAND "<SOURCE_DIR>/configure -confirm-license  -opensource -nomake examples \
                -no-pch -no-dbus -no-icu -no-iconv -no-openssl -no-nis \
                -no-compile-examples -qt-freetype -qt-harfbuzz -opengl desktop \
                -qt-zlib -qt-pcre -qt-libpng -qt-libjpeg 
                -skip qt3d -skip qtactiveqt -skip qtcanvas3d -skip qtconnectivity \
                -skip qtenginio -skip qtgraphicaleffects -skip qtlocation -skip qtmultimedia \
                -skip qtwayland -skip qtwebchannel -skip qtwebengine -skip qtwebsockets \
                -skip qtwebview -skip qtxmlpatterns \
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
            URL http://download.qt.io/development_releases/qt/5.6/5.6.0-beta/single/qt-everywhere-opensource-src-5.6.0-beta.tar.gz
            URL_MD5 d0c366ddf9796256e168ccce6b087175
            INSTALL_DIR ${EXTPREFIX_qt}
            CONFIGURE_COMMAND "<SOURCE_DIR>/configure -prefix ${EXTPREFIX_qt} -I ${EXTPREFIX_qt}/include -L ${EXTPREFIX_qt}/lib  \
            -opensource -confirm-license -nomake examples \
            -no-sql-sqlite -no-openssl -no-qml-debug -no-mtdev  -no-journald  -no-syslog -no-glib  -no-pulseaudio -no-alsa  -no-gtkstyle  \
            -no-nis  -no-cups  -no-iconv   -no-tslib   -no-icu  -no-directfb   -no-linuxfb  -no-gstreamer -no-pch \
            -qt-zlib -qt-pcre -no-icu -qt-libpng -qt-libjpeg  -qt-harfbuzz  -no-libproxy -qt-freetype -qt-xcb -qt-xkbcommon-x11 \
            -optimized-qmake  \
            -skip qt3d -skip qtactiveqt -skip qtcanvas3d -skip qtconnectivity \
            -skip qtenginio -skip qtgraphicaleffects -skip qtlocation \
            -skip qtmultimedia -skip qtwayland -skip qtwebchannel \
            -skip qtwebengine -skip qtwebsockets -skip qtwebview \
            -skip qtxmlpatterns"
            
            BUILD_COMMAND gmake
            INSTALL_COMMAND gmake install
            UPDATE_COMMAND ""
            BUILD_IN_SOURCE 1
            ALWAYS 0
        )
    endif()
endif()
