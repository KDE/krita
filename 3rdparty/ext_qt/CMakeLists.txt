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
      set(QT_EXTRA_DEPS "")
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
      set(QT_EXTRA_DEPS ext_jom)
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

    # Custom ANGLE
    set(QT_EXTRA_DEPS ${QT_EXTRA_DEPS} ext_googleangle)

  ExternalProject_Add(
      ext_qt
      DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
      GIT_REPOSITORY https://invent.kde.org/szaman/qt5.git
      GIT_TAG krita/5.15
      GIT_SUBMODULES qtbase qtdeclarative qttools qtmultimedia qtsvg qtwinextras
      GIT_SUBMODULES_RECURSE false
      GIT_SHALLOW true
      GIT_PROGRESS true
      GIT_REMOTE_UPDATE_STRATEGY REBASE

      INSTALL_DIR ${EXTPREFIX_qt}
      CONFIGURE_COMMAND <SOURCE_DIR>/configure.bat ${_QT_conf}

      BUILD_COMMAND ${QT_BUILD_COMMAND}

      INSTALL_COMMAND ${QT_INSTALL_COMMAND}
      UPDATE_COMMAND ""
      # Use a short name to reduce the chance of exceeding path length limit
      SOURCE_DIR s
      BINARY_DIR b
      DEPENDS ext_patch ${OPENSSL_DEP} ${QT_EXTRA_DEPS} ${ZLIB_DEP}
  )
elseif (ANDROID)
  ExternalProject_Add(
      ext_qt
      DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
      URL http://download.qt.io/archive/qt/5.12/5.12.12/single/qt-everywhere-src-5.12.12.tar.xz
      URL_MD5 88e7a1c9952bf5732751c794716c6a55

      PATCH_COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Fix-QLocale-system-and-uiLanguages-for-the-mobile-pl.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0002-Bugfix-fix-the-offset-bug-when-using-Stylus-with-And.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0003-Add-support-for-pen-tilt-rotation-for-Android.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0004-Pass-keyboard-modifiers-for-mouse-and-tablet-events.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0006-Add-file-engine-for-Android-content-URLs.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0007-Android-handle-check-permissions-before-any-file-ope.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0008-Android-Add-support-for-getting-information-about-co.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0009-Also-fall-back-to-direct-write-for-Android-content-U.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0010-Android-find-column-index-at-runtime.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0011-Android-return-the-actual-file-name-the-URI-is-point.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0012-Android-Don-t-hide-Menubar-even-if-MenuKey-is-presen.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0013-Emulate-Long-Press-as-Right-Click-on-Android.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0014-platform-android-Add-native-file-dialog.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0015-Android-Fix-native-open-fileDialog-crash-on-Android.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0016-Android-Fix-native-fileDialog-not-waiting-for-user-s.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0017-Android-fully-integrate-native-file-dialog.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0018-Android-fix-mimeTypes-handling-for-file-dialog.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0019-Android-allow-file-dialog-to-use-remote-locations.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0020-Android-Clear-the-m_selectedFile-variable-when-showi.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0021-Android-Only-emit-the-signals-once-when-doing-multip.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0022-Android-Properly-close-the-File-Descriptor.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0023-Android-Workarounds-to-explicitly-handle-content-URI.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0024-Android-Add-clipboard-buffer-for-mimeTypes-that-aren.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0025-Android-Move-events-handling-to-QtLayout.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0026-Android-Use-processOpenModeFlags-to-get-right-OpenMo.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0027-Android-use-EXTRA_INITIAL_URI-as-the-defaultDir.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0028-Android-Pass-the-button-state-through-JNI.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0029-Android-Handle-the-ACTION_CANCEL-Event-from-Android.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0030-Android-prevent-out-of-order-events.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0031-Android-Properly-handle-Tablet-events.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0033-Android-Add-a-way-to-check-permissions-for-external-.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0034-Android-Give-preference-to-certain-WindowTypes.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0035-Android-check-if-the-Uri-is-writable.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0037-Android-Don-t-invoke-Qt-s-exit-path.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0038-Android-Add-window-manager-for-raster-based-QWidgets.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0039-Fix-Text-select-handle-misplaced-on-fields-inside-QD.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0041-Android-Use-External-Storage-for-some-Standard-Paths.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0042-Android-Disable-QTemporaryFile-usage-in-QFile-copy.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0043-Android-Don-t-use-QDir-cleanPath-for-content-Uri.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0044-Android-Rework-Storage-Access-Framework.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0045-Android-Add-support-for-changing-cursor.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0046-Android-Update-clang-mkspecs.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Disable-QAccessibility-by-not-traversing-nodes.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Android-Rename-rs-locale-to-se.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Hack-Workaround-the-absence-of-FLAG_SUPPORTS_WRITE-o.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Android-Make-window-which-is-clicked-on-the-activate.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Android-Make-window-manager-a-bit-more-distinguishab.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Android-setBackingStore-of-a-window-if-the-platformW.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Android-Fix-incorrect-handling-of-window-modality.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Android-Introducing-QtInputEventDispatcher.patch

            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0111-Fix-unbalanced-KeyPress-Release-events-in-children-o.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0112-Fix-shortcuts-assigned-to-special-keys-on-non-latin-.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0113-Fix-multiple-shortcuts-match-from-different-keyboard.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Remove-chunk-size-limits.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0120-qtbase-Fix-build-with-GCC-11.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtdeclarative -i ${CMAKE_CURRENT_SOURCE_DIR}/0121-Fix-compilation-of-QtDeclarative-with-GCC11.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtdeclarative -i ${CMAKE_CURRENT_SOURCE_DIR}/0122-Don-t-use-UINT_MAX-as-invalid-array-index-in-PropertyKey.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtdeclarative -i ${CMAKE_CURRENT_SOURCE_DIR}/0124-qtdeclarative-Include-limits-in-yarr-h.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtdeclarative -i ${CMAKE_CURRENT_SOURCE_DIR}/0125-qtdeclarative-Build-fixes-for-GCC-11.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0126-qtbase-Build-fixes-for-GCC-11.patch
            COMMAND ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0131-Fix-build-with-Android-NDK-21.4.patch

            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Do-not-crash-if-the-foundry-name-is-an-empty-space.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Check-if-we-reached-the-end-of-Iterator.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0100-Fix-artifacts-when-rendering-multisubpath-dashed-QPa.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/bug449118-QMdiSubWindow-Set-correct-ControlLabel-size.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/bug449779-Check-for-uninitialized-QDockAreaLayoutInfo.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtdeclarative -i ${CMAKE_CURRENT_SOURCE_DIR}/Make-QQuickWindow-renderer-follow-real-logical-size.patch

            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/frac_dpi_paint_artifacts/0001-Add-faster-path-for-scaling-QRegion-with-multiple-re.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/frac_dpi_paint_artifacts/0002-Fix-QPainter-off-by-one-clipping-for-some-non-intege.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/frac_dpi_paint_artifacts/0003-Fix-under-non-integer-scaling-incorrect-update-area.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-QMenu-make-less-sensitive-to-mouse-jitter.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Android-Set-TMPDIR-to-point-to-cache-dir-rather-than.patch



      CONFIGURE_COMMAND <SOURCE_DIR>/configure -prefix ${EXTPREFIX_qt} -opensource -confirm-license -verbose -nomake examples -nomake tests -nomake tools -skip qt3d -skip qtactiveqt -skip qtcanvas3d -skip qtconnectivity -skip qtlocation -skip qtwayland -skip qtwebchannel -skip qtwebengine -skip qtwebsockets -skip qtwebview -skip qtserialport -skip qtdatavis3d -skip qtvirtualkeyboard -skip qtspeech -skip qtsensors -skip qtgamepad -skip qtscxml -skip qtremoteobjects -skip qtxmlpatterns -skip qtnetworkauth  -skip qtcharts -skip qtdatavis3d -skip qtgamepad -skip qtpurchasing -skip qtscxml -skip qtserialbus -skip qtspeech -skip qtvirtualkeyboard -android-sdk ${ANDROID_SDK_ROOT} -android-ndk ${CMAKE_ANDROID_NDK} -android-arch ${ANDROID_ABI} -xplatform android-clang -android-ndk-platform android-23 -make libs -qt-sqlite -openssl-linked OPENSSL_PREFIX=${EXTPREFIX_qt} -system-zlib

      BUILD_COMMAND ${QT_BUILD_COMMAND}
      INSTALL_COMMAND ${QT_INSTALL_COMMAND}

      INSTALL_DIR ${EXTPREFIX_qt}


      UPDATE_COMMAND ""
      BUILD_IN_SOURCE 1

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
      URL http://download.qt.io/archive/qt/5.12/5.12.12/single/qt-everywhere-src-5.12.12.tar.xz
      URL_MD5 88e7a1c9952bf5732751c794716c6a55

      PATCH_COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0012-Synthesize-Enter-LeaveEvent-for-accepted-QTabletEven.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0013-Poison-Qt-s-headers-with-a-mark-about-presence-of-En.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0100-Fix-artifacts-when-rendering-multisubpath-dashed-QPa.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0101-Don-t-eat-ShortcutOverride-events-when-there-is-a-pa.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0110-Pass-proper-dirty-rect-in-paint-event-to-QOpenGLWidg.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0111-Fix-unbalanced-KeyPress-Release-events-in-children-o.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0112-Fix-shortcuts-assigned-to-special-keys-on-non-latin-.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0113-Fix-multiple-shortcuts-match-from-different-keyboard.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0114-Fix-shortcuts-with-special-keys-on-non-US-layouts-on.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Check-if-we-reached-the-end-of-Iterator.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0116-Make-Qt-relocatable.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0119-Fix-relocated-Qt-to-be-found-when-its-path-has-non-A.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0120-qtbase-Fix-build-with-GCC-11.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtdeclarative -i ${CMAKE_CURRENT_SOURCE_DIR}/0121-Fix-compilation-of-QtDeclarative-with-GCC11.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtdeclarative -i ${CMAKE_CURRENT_SOURCE_DIR}/0122-Don-t-use-UINT_MAX-as-invalid-array-index-in-PropertyKey.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtdeclarative -i ${CMAKE_CURRENT_SOURCE_DIR}/0124-qtdeclarative-Include-limits-in-yarr-h.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtdeclarative -i ${CMAKE_CURRENT_SOURCE_DIR}/0125-qtdeclarative-Build-fixes-for-GCC-11.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0126-qtbase-Build-fixes-for-GCC-11.patch
            COMMAND ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0131-Fix-build-with-Android-NDK-21.4.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtdeclarative -i ${CMAKE_CURRENT_SOURCE_DIR}/0134-V4-Generate-function-tables-on-64bit-windows.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtdeclarative -i ${CMAKE_CURRENT_SOURCE_DIR}/0135-Use-lowercase-name-for-window-header.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtdeclarative -i ${CMAKE_CURRENT_SOURCE_DIR}/0136-JIT-When-making-memory-writable-include-the-exceptio.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtdeclarative -i ${CMAKE_CURRENT_SOURCE_DIR}/0137-masm-Add-error-handling-for-failed-mprotect.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtdeclarative -i ${CMAKE_CURRENT_SOURCE_DIR}/0138-Fix-Clang-10-warning-about-converting-ULLONG_MAX-to-.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtdeclarative -i ${CMAKE_CURRENT_SOURCE_DIR}/0139-Fix-Wdeprecated-copy-warnings.patch

            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Do-not-crash-if-the-foundry-name-is-an-empty-space.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/bug449118-QMdiSubWindow-Set-correct-ControlLabel-size.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/bug449779-Check-for-uninitialized-QDockAreaLayoutInfo.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtdeclarative -i ${CMAKE_CURRENT_SOURCE_DIR}/Make-QQuickWindow-renderer-follow-real-logical-size.patch

            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/frac_dpi_paint_artifacts/0001-Add-faster-path-for-scaling-QRegion-with-multiple-re.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/frac_dpi_paint_artifacts/0002-Fix-QPainter-off-by-one-clipping-for-some-non-intege.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/frac_dpi_paint_artifacts/0003-Fix-under-non-integer-scaling-incorrect-update-area.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-QMenu-make-less-sensitive-to-mouse-jitter.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Fix-unbalanced-KeyPress-Release-on-non-latin-keyboar.patch

      CMAKE_ARGS -DOPENSSL_LIBS='-L${EXTPREFIX_qt}/lib -lssl -lcrypto'

      CONFIGURE_COMMAND <SOURCE_DIR>/configure -prefix ${EXTPREFIX_qt} -opensource -confirm-license -feature-relocatable -openssl-linked -verbose -nomake examples -skip qt3d -skip qtactiveqt -skip qtcanvas3d -skip qtconnectivity -skip qtlocation -skip qtwayland -skip qtwebchannel -skip qtwebengine -skip qtwebsockets -skip qtwebview -skip qtandroidextras -skip qtserialport -skip qtdatavis3d -skip qtvirtualkeyboard -skip qtspeech -skip qtsensors -skip qtgamepad -skip qtscxml -skip qtremoteobjects -skip qtxmlpatterns -skip qtnetworkauth  -skip qtcharts -skip qtdatavis3d -skip qtgamepad -skip qtpurchasing -skip qtscxml -skip qtserialbus -skip qtspeech -skip qtvirtualkeyboard -qt-sqlite -system-zlib ${QT_DEBUG_OPTIONS}

      BUILD_COMMAND ${QT_BUILD_COMMAND}
      INSTALL_COMMAND ${QT_INSTALL_COMMAND}

      INSTALL_DIR ${EXTPREFIX_qt}


      UPDATE_COMMAND ""
      BUILD_IN_SOURCE 1
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
