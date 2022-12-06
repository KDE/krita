SET(EXTPREFIX_qt  "${EXTPREFIX}")

if (MINGW)
  set(QT_BUILD_COMMAND   mingw32-make -j${SUBMAKE_JOBS})
  set(QT_INSTALL_COMMAND mingw32-make -j${SUBMAKE_JOBS} install)
elseif (MSVC)
  set(QT_BUILD_COMMAND jom -j${SUBMAKE_JOBS})
  set(QT_INSTALL_COMMAND jom -j${SUBMAKE_JOBS} install)
elseif (CMAKE_GENERATOR MATCHES "Makefiles")
  set (QT_BUILD_COMMAND $(MAKE) -j${SUBMAKE_JOBS})
  set (QT_INSTALL_COMMAND $(MAKE) install)
else()
  set (QT_BUILD_COMMAND make -j${SUBMAKE_JOBS})
  set (QT_INSTALL_COMMAND make install)
endif()

if (WIN32)

    if(MINGW)
      if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(QT_CONFIG_PLATFORM win32-clang-g++)
      else()
        set(QT_CONFIG_PLATFORM win32-g++)
      endif()
    elseif(MSVC)
      ExternalProject_Add(ext_jom
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://download.qt.io/official_releases/jom/jom_1_1_3.zip
        URL_HASH SHA256=128fdd846fe24f8594eed37d1d8929a0ea78df563537c0c1b1861a635013fff8

        BUILD_COMMAND ${CMAKE_COMMAND} -E echo Deploying Jom
        CONFIGURE_COMMAND ""
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/jom.exe ${EXTPREFIX_qt}/bin

        UPDATE_COMMAND ""
      )

      set(QT_CONFIG_PLATFORM win32-msvc)
      list(APPEND QT_EXTRA_DEPS ext_jom)
    endif(MINGW)

    list(APPEND _QT_conf -skip qt3d -skip qtactiveqt -skip qtcanvas3d
        -skip qtconnectivity -skip qtdoc
        -skip qtlocation -skip qtsensors -skip qtserialport -skip qtwayland
        -skip qtwebchannel -skip qtwebengine -skip qtwebsockets -skip qtwebview
        -skip qtxmlpatterns -nomake examples -nomake tools
        -no-compile-examples -no-dbus -no-iconv -no-qml-debug
        -no-libproxy -no-system-proxies -no-icu -no-mtdev
        -skip qtcharts -skip qtdatavis3d -skip qtgamepad -skip qtnetworkauth
        -skip qtpurchasing -skip qtremoteobjects -skip qtscxml -skip qtserialbus
        -skip qtspeech -skip qtvirtualkeyboard -qt-sqlite
        #
        -system-zlib -qt-pcre -qt-libpng -qt-libjpeg -openssl-linked -qt-freetype -qt-harfbuzz
        -I ${EXTPREFIX_qt}/include
        -L ${EXTPREFIX_qt}/lib
        #
        -opensource -confirm-license
        -feature-relocatable
        -no-feature-d3d12
        -no-feature-wmf
        -no-feature-qdoc
        -no-direct2d
        #
        -release -platform ${QT_CONFIG_PLATFORM} -prefix ${EXTPREFIX_qt}
        QMAKE_LFLAGS_APP+=${SECURITY_EXE_LINKER_FLAGS}
        QMAKE_LFLAGS_SHLIB+=${SECURITY_SHARED_LINKER_FLAGS}
        QMAKE_LFLAGS_SONAME+=${SECURITY_SHARED_LINKER_FLAGS}
    )
    if (MSVC)
      list(APPEND _QT_conf
        QMAKE_CFLAGS+=${SECURITY_C_FLAGS}
        QMAKE_CXXFLAGS+=${SECURITY_CXX_FLAGS}
      )
    endif (MSVC)

    if(QT_ENABLE_DEBUG_INFO OR WIN32)
        # Set the option to build Qt with debugging info enabled
        list(APPEND _QT_conf -force-debug-info)
    endif(QT_ENABLE_DEBUG_INFO OR WIN32)
    if (QT_ENABLE_ASAN)
        list(APPEND _QT_conf
            QMAKE_CFLAGS+=-fsanitize=address
            QMAKE_CXXFLAGS+=-fsanitize=address
            QMAKE_LFLAGS+=-fsanitize=address
            )
    endif ()
    if (MINGW)
        if (NOT QT_ENABLE_DEBUG_INFO)
            # Generate minimal debug info for backtraces
            list(APPEND _QT_conf
                QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO-=-g
                QMAKE_CFLAGS+=-g1
                QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO-=-g
                QMAKE_CXXFLAGS+=-g1
            )
            if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
                # This is needed for proper C++ function signatures
                list(APPEND _QT_conf
                    QMAKE_CFLAGS+=-fdebug-info-for-profiling
                    QMAKE_CXXFLAGS+=-fdebug-info-for-profiling
                )
            endif ()
        endif ()
        if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            # Workaround for DrMingw, see parent CMakeLists.txt
            list(APPEND _QT_conf
                QMAKE_CFLAGS+=-gdwarf-aranges
                QMAKE_CXXFLAGS+=-gdwarf-aranges
            )
        endif ()
    elseif (MSVC)
      if (NOT QT_ENABLE_DEBUG_INFO)
        # Generate minimal debug info for backtraces
        list(APPEND _QT_conf
            QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO-=-Zi
            QMAKE_CFLAGS+=-Zi
            QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO-=-Zi
            QMAKE_CXXFLAGS+=-Zi
        )
      endif ()
    endif ()
    list(APPEND _QT_conf -opengl dynamic)

    if (USE_STRAWBERRYPERL)
      list(APPEND QT_EXTRA_DEPS ext_strawberryperl)
    endif()

    if (KRITA_PERL_PATH) 
      list(APPEND QT_ENV_ARGS --modify PATH=path_list_append:${KRITA_PERL_PATH})
    else()
      message(WARNING "KRITA_PERL_PATH is not set. Most probably, Qt will fail to configure!")
    endif()

  ExternalProject_Add(
      ext_qt
      DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
      GIT_REPOSITORY https://invent.kde.org/szaman/qt5.git
      GIT_TAG krita/5.15
      GIT_SUBMODULES qtbase qtdeclarative qttools qtmultimedia qtsvg qtwinextras qttranslations
      GIT_SUBMODULES_RECURSE false
      GIT_SHALLOW true
      GIT_PROGRESS true
      GIT_REMOTE_UPDATE_STRATEGY REBASE

      INSTALL_DIR ${EXTPREFIX_qt}
      CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env ${QT_ENV_ARGS} <SOURCE_DIR>/configure.bat ${_QT_conf}

      BUILD_COMMAND ${CMAKE_COMMAND} -E env ${QT_ENV_ARGS} ${QT_BUILD_COMMAND}

      INSTALL_COMMAND ${CMAKE_COMMAND} -E env ${QT_ENV_ARGS} ${QT_INSTALL_COMMAND}
      UPDATE_COMMAND ""
      # Use a short name to reduce the chance of exceeding path length limit
      SOURCE_DIR s
      BINARY_DIR b
      DEPENDS ext_patch ext_googleangle ${OPENSSL_DEP} ${QT_EXTRA_DEPS} ${ZLIB_DEP} 
  )
elseif (ANDROID)
  ExternalProject_Add(
      ext_qt
      DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
      GIT_REPOSITORY https://invent.kde.org/szaman/qt5.git
      GIT_TAG krita/5.15
      GIT_SUBMODULES qtbase qtdeclarative qttools qtmultimedia qtsvg qttranslations qtandroidextras
      GIT_SUBMODULES_RECURSE false
      GIT_SHALLOW true
      GIT_PROGRESS true
      GIT_REMOTE_UPDATE_STRATEGY REBASE

      # TODO(sh_zam): We can probably skip the skipping now.
      CONFIGURE_COMMAND <SOURCE_DIR>/configure -prefix ${EXTPREFIX_qt} -opensource -confirm-license -verbose
          -android-sdk ${ANDROID_SDK_ROOT} -android-ndk ${CMAKE_ANDROID_NDK} -android-arch ${ANDROID_ABI}
          -xplatform android-clang -android-ndk-platform android-23
          -make libs -qt-sqlite -openssl-linked OPENSSL_PREFIX=${EXTPREFIX_qt} -system-zlib
          -nomake examples -nomake tests -nomake tools -skip qt3d -skip qtactiveqt -skip qtcanvas3d
          -skip qtconnectivity -skip qtlocation -skip qtwayland -skip qtwebchannel -skip qtwebengine
          -skip qtwebsockets -skip qtwebview -skip qtserialport  -skip qtvirtualkeyboard -skip qtspeech
          -skip qtsensors  -skip qtscxml -skip qtremoteobjects -skip qtxmlpatterns -skip qtnetworkauth
          -skip qtcharts -skip qtdatavis3d -skip qtgamepad -skip qtpurchasing  -skip qtserialbus

      BUILD_COMMAND ${QT_BUILD_COMMAND}
      INSTALL_COMMAND ${QT_INSTALL_COMMAND}

      INSTALL_DIR ${EXTPREFIX_qt}

      UPDATE_COMMAND ""
      BUILD_IN_SOURCE 0

      DEPENDS ext_openssl ${ZLIB_DEP}
)
elseif (NOT APPLE)
    if (QT_ENABLE_DEBUG_INFO)
        # Set the option to build Qt with debugging info enabled
        set(QT_DEBUG_OPTIONS -force-debug-info)
    endif()
    if (QT_ENABLE_ASAN)
        list(APPEND _QT_conf
            QMAKE_CFLAGS+=-fsanitize=address
            QMAKE_CXXFLAGS+=-fsanitize=address
            QMAKE_LFLAGS+=-fsanitize=address
            )
    endif ()


    ExternalProject_Add(
      ext_qt
      DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
      GIT_REPOSITORY https://invent.kde.org/szaman/qt5.git
      GIT_TAG krita/5.15
      GIT_SUBMODULES qtbase qtdeclarative qttools qtmultimedia qtsvg qtx11extras qttranslations
      GIT_SUBMODULES_RECURSE false
      GIT_SHALLOW true
      GIT_PROGRESS true
      GIT_REMOTE_UPDATE_STRATEGY REBASE

      CMAKE_ARGS -DOPENSSL_LIBS='-L${EXTPREFIX_qt}/lib -lssl -lcrypto'

      CONFIGURE_COMMAND <SOURCE_DIR>/configure -prefix ${EXTPREFIX_qt} -opensource -confirm-license -feature-relocatable -openssl-linked -verbose -nomake examples -skip qt3d -skip qtactiveqt -skip qtcanvas3d -skip qtconnectivity -skip qtlocation -skip qtwayland -skip qtwebchannel -skip qtwebengine -skip qtwebsockets -skip qtwebview -skip qtandroidextras -skip qtserialport -skip qtdatavis3d -skip qtvirtualkeyboard -skip qtspeech -skip qtsensors -skip qtgamepad -skip qtscxml -skip qtremoteobjects -skip qtxmlpatterns -skip qtnetworkauth  -skip qtcharts -skip qtdatavis3d -skip qtgamepad -skip qtpurchasing -skip qtscxml -skip qtserialbus -skip qtspeech -skip qtvirtualkeyboard -qt-sqlite -system-zlib ${QT_DEBUG_OPTIONS}

      BUILD_COMMAND ${QT_BUILD_COMMAND}
      INSTALL_COMMAND ${QT_INSTALL_COMMAND}

      INSTALL_DIR ${EXTPREFIX_qt}


      UPDATE_COMMAND ""
      BUILD_IN_SOURCE OFF
      DEPENDS ${ZLIB_DEP}
  )
else( APPLE )

      if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "arm64")
        set(QT_DEVICE_OPTION "-device-option" "QMAKE_APPLE_DEVICE_ARCHS=arm64")
      endif()

      ExternalProject_Add(
        ext_qt
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL http://download.qt.io/archive/qt/5.12/5.12.12/single/qt-everywhere-src-5.12.12.tar.xz
        URL_MD5 88e7a1c9952bf5732751c794716c6a55

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0012-Synthesize-Enter-LeaveEvent-for-accepted-QTabletEven.patch
                COMMAND ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0013-Poison-Qt-s-headers-with-a-mark-about-presence-of-En.patch
                COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0100-Fix-artifacts-when-rendering-multisubpath-dashed-QPa.patch
                COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0101-Don-t-eat-ShortcutOverride-events-when-there-is-a-pa.patch
                COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0110-Pass-proper-dirty-rect-in-paint-event-to-QOpenGLWidg.patch
                COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0111-Fix-unbalanced-KeyPress-Release-events-in-children-o.patch
                COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0112-Fix-shortcuts-assigned-to-special-keys-on-non-latin-.patch
                COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0113-Fix-multiple-shortcuts-match-from-different-keyboard.patch
                COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Remove-chunk-size-limits.patch
                COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Check-if-we-reached-the-end-of-Iterator.patch
                COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Do-not-crash-if-the-foundry-name-is-an-empty-space.patch
                COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Add-missing-macOS-header.patch
                COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/bug449118-QMdiSubWindow-Set-correct-ControlLabel-size.patch
                COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/bug449779-Check-for-uninitialized-QDockAreaLayoutInfo.patch
                COMMAND  ${PATCH_COMMAND} -p1 -d qtdeclarative -i ${CMAKE_CURRENT_SOURCE_DIR}/Make-QQuickWindow-renderer-follow-real-logical-size.patch

                COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/frac_dpi_paint_artifacts/0001-Add-faster-path-for-scaling-QRegion-with-multiple-re.patch
                COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/frac_dpi_paint_artifacts/0002-Fix-QPainter-off-by-one-clipping-for-some-non-intege.patch
                COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/frac_dpi_paint_artifacts/0003-Fix-under-non-integer-scaling-incorrect-update-area.patch
                COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-QMenu-make-less-sensitive-to-mouse-jitter.patch
                COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0128-macOS-Do-not-append-defaultSuffix-in-save-dialog.patch

        CMAKE_ARGS -DOPENSSL_LIBS='-L${EXTPREFIX_qt}/lib -lssl -lcrypto'

        INSTALL_DIR ${EXTPREFIX_qt}
        CONFIGURE_COMMAND <SOURCE_DIR>/configure
        -skip qt3d -skip qtactiveqt -skip qtcanvas3d -skip qtconnectivity -skip qtdoc -skip qtlocation -skip qtsensors -skip qtserialport -skip qtwayland
            -skip qtwebchannel -skip qtwebsockets -skip qtwebview -skip qtwebengine -skip qtxmlpatterns
            -skip qtcharts -skip qtdatavis3d  -skip qtgamepad -skip qtnetworkauth
            -skip qtpurchasing -skip qtremoteobjects -skip qtscxml -skip qtserialbus -skip qtspeech -skip qtvirtualkeyboard -nomake examples -nomake tools -no-compile-examples
            -no-dbus -no-iconv -no-qml-debug -no-libproxy -no-system-proxies -no-icu -no-mtdev -system-zlib -qt-pcre -qt-sqlite
            -opensource -confirm-license  -openssl-linked -prefix ${EXTPREFIX_qt}
            ${QT_DEVICE_OPTION}

        BUILD_COMMAND ${QT_BUILD_COMMAND}
        INSTALL_COMMAND ${QT_INSTALL_COMMAND}

        UPDATE_COMMAND ""
        BUILD_IN_SOURCE 1
        DEPENDS ${ZLIB_DEP}
      )
endif()
