SET(EXTPREFIX_qt  "${EXTPREFIX}")
if (WIN32)

    if(MINGW)
      if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(QT_CONFIG_PLATFORM win32-clang-g++)
      else()
        set(QT_CONFIG_PLATFORM win32-g++)
      endif()
      set(QT_BUILD_COMMAND   mingw32-make -j${SUBMAKE_JOBS})
      set(QT_INSTALL_COMMAND mingw32-make -j${SUBMAKE_JOBS} install)
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
      set(QT_BUILD_COMMAND jom -j${SUBMAKE_JOBS})
      set(QT_INSTALL_COMMAND jom -j${SUBMAKE_JOBS} install)
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
    if (QT_ENABLE_DEBUG_INFO)
        # Set the option to build Qt with debugging info enabled
        list(APPEND _QT_conf -force-debug-info)
        if (MINGW)
            if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
                list(APPEND _QT_conf
                    QMAKE_CFLAGS+=-gdwarf-aranges
                    QMAKE_CXXFLAGS+=-gdwarf-aranges
                )
            endif ()
        endif ()
    endif(QT_ENABLE_DEBUG_INFO)
    if (QT_ENABLE_DYNAMIC_OPENGL)
        list(APPEND _QT_conf -opengl dynamic -angle)
    else (QT_ENABLE_DYNAMIC_OPENGL)
        list(APPEND _QT_conf -opengl desktop -no-angle)
    endif (QT_ENABLE_DYNAMIC_OPENGL)

    # MIME-type optimization patches
    set(ext_qt_PATCH_COMMAND
                     ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Use-fast-path-for-unsupported-mime-types.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0002-Hack-always-return-we-support-DIBV5.patch
            )

    # Tablet support patches
    if (NOT USE_QT_TABLET_WINDOWS)
        set(ext_qt_PATCH_COMMAND ${ext_qt_PATCH_COMMAND}
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-disable-wintab.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/disable-winink.patch
            )
    else()
        set(ext_qt_PATCH_COMMAND ${ext_qt_PATCH_COMMAND}
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0020-Synthesize-Enter-LeaveEvent-for-accepted-QTabletEven.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0023-Implement-a-switch-for-tablet-API-on-Windows.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0024-Fetch-stylus-button-remapping-from-WinTab-driver.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0025-Disable-tablet-relative-mode-in-Qt.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0026-Fetch-mapped-screen-size-from-the-Wintab-driver.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0027-Switch-stylus-pointer-type-when-the-tablet-is-in-the.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0028-Fix-updating-tablet-pressure-resolution-on-every-pro.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0029-Add-a-workaround-for-button-mapping-on-Lenovo-Yoga-C.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0030-Make-sure-if-WinInk-is-correctly-initialized-if-wint.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0051-Add-workaround-for-handling-table-press-correctly-in.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0052-WinInk-Synthesize-mouse-events-for-full-pen-stroke.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0053-WinInk-Prevent-Leave-event-confusing-mouse-capture-s.patch
            )
    endif()

    # HDR patches
    set(ext_qt_PATCH_COMMAND ${ext_qt_PATCH_COMMAND}
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0003-Implement-openGL-surface-color-space-selection-in-An.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0004-Implement-color-space-selection-for-QSurfaceFormat.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0005-Implement-color-conversion-for-the-backing-store-tex.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0006-Return-QScreen-s-HMONITOR-handle-via-QPlatformNative.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0007-Implement-a-manual-test-for-checking-is-HDR-features.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0008-Fix-notification-of-QDockWidget-when-it-gets-undocke.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0009-Fix-Rec2020-display-format.patch
            )

    # Other patches
    set(ext_qt_PATCH_COMMAND ${ext_qt_PATCH_COMMAND}
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0060-Windows-Add-a-default-setting-for-hasBorderInFullScr.patch
        #COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0061-Hack-to-hide-1px-border-with-OpenGL-fullscreen-hack.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qttools -i ${CMAKE_CURRENT_SOURCE_DIR}/windeployqt-force-allow-debug-info.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0080-Sync-buffers-of-the-destination-file-after-QFile-cop.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0081-Fix-no-warning-for-overwriting-files-in-non-native-d.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0082-Make-jp-e-g-default-extensions-context-aware.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0100-Fix-artifacts-when-rendering-multisubpath-dashed-QPa.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0101-Don-t-eat-ShortcutOverride-events-when-there-is-a-pa.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0110-Pass-proper-dirty-rect-in-paint-event-to-QOpenGLWidg.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0111-Fix-unbalanced-KeyPress-Release-events-in-children-o.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0112-Fix-shortcuts-assigned-to-special-keys-on-non-latin-.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0113-Fix-multiple-shortcuts-match-from-different-keyboard.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0115-Fix-fallback-latin-shortcuts-not-to-be-the-top-prior.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Remove-chunk-size-limits.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Check-whether-there-is-a-QGestureRecognizer-at-all.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Check-if-we-reached-the-end-of-Iterator.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0116-Make-Qt-relocatable.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qttools -i ${CMAKE_CURRENT_SOURCE_DIR}/0117-windeployqt-Warn-only-in-case-patching-of-Qt5Core-fails.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qttools -i ${CMAKE_CURRENT_SOURCE_DIR}/0118-Don-t-patch-QtCore-when--feature-relocatable-is-enabled.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0119-Fix-relocated-Qt-to-be-found-when-its-path-has-non-A.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/windows_frac_dpi/0001-Compute-logical-DPI-on-a-per-screen-basis.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/windows_frac_dpi/0002-Update-Dpi-and-scale-factor-computation.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/windows_frac_dpi/0003-Move-QT_FONT_DPI-to-cross-platform-code.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/windows_frac_dpi/0004-Update-QT_SCREEN_SCALE_FACTORS.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/windows_frac_dpi/0005-Deprecate-QT_AUTO_SCREEN_SCALE_FACTOR.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/windows_frac_dpi/0006-Add-high-DPI-scale-factor-rounding-policy-C-API.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/windows_frac_dpi/0007-QtGui-Refactor-parsing-of-the-High-DPI-scaling-env-v.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/windows_frac_dpi/0008-QHighDpiScaling-fix-potential-null-pointer-dereferen.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/windows_frac_dpi/0009-QHighDpiScaling-impove-readability-of-screenSubfacto.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/windows_frac_dpi/0010-Windows-Add-support-for-PerMonitorV2-DPI-awareness.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/windows_frac_dpi/0011-Windows-Change-default-to-ProcessPerMonitorV2DpiAwar.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0120-qtbase-Fix-build-with-GCC-11.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtdeclarative -i ${CMAKE_CURRENT_SOURCE_DIR}/0121-Fix-compilation-of-QtDeclarative-with-GCC11.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtdeclarative -i ${CMAKE_CURRENT_SOURCE_DIR}/0122-Don-t-use-UINT_MAX-as-invalid-array-index-in-PropertyKey.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0123-Fix-Angle-build-with-C-11.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtdeclarative -i ${CMAKE_CURRENT_SOURCE_DIR}/0124-qtdeclarative-Include-limits-in-yarr-h.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtdeclarative -i ${CMAKE_CURRENT_SOURCE_DIR}/0125-qtdeclarative-Build-fixes-for-GCC-11.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0126-qtbase-Build-fixes-for-GCC-11.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0127-Fix-finding-zlib-for-mingw-using-libzlib.dll.a.patch


        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Do-not-crash-if-the-foundry-name-is-an-empty-space.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/bug449118-QMdiSubWindow-Set-correct-ControlLabel-size.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/bug449779-Check-for-uninitialized-QDockAreaLayoutInfo.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtdeclarative -i ${CMAKE_CURRENT_SOURCE_DIR}/Make-QQuickWindow-renderer-follow-real-logical-size.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/QTBUG-94031-Fix-compile-with-MinGW-W64-9.0.0-Redefinition-of-str.patch

        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/frac_dpi_paint_artifacts/0001-Add-faster-path-for-scaling-QRegion-with-multiple-re.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/frac_dpi_paint_artifacts/0002-Fix-QPainter-off-by-one-clipping-for-some-non-intege.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/frac_dpi_paint_artifacts/0003-Fix-under-non-integer-scaling-incorrect-update-area.patch

        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/misc_hidpi_backport/0002-Windows-QPA-Fix-missing-resize-when-changing-the-sca.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/misc_hidpi_backport/0003-qwindowswindow-Fix-screen-changes-between-different-.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/misc_hidpi_backport/0004-Windows-QPA-Fix-geometry-when-firing-a-full-expose.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/misc_hidpi_backport/0005-Windows-QPA-Fix-restoring-from-fullscreen-in-High-DP.patch

        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Return-0-for-Window-if-there-is-no-widget.patch
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-QMenu-make-less-sensitive-to-mouse-jitter.patch
    )


  ExternalProject_Add(
      ext_qt
      DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
      URL http://download.qt.io/archive/qt/5.12/5.12.12/single/qt-everywhere-src-5.12.12.zip
      URL_MD5 5317551ce668830bfa2404346aec2f16

      PATCH_COMMAND ${ext_qt_PATCH_COMMAND}

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



      CONFIGURE_COMMAND <SOURCE_DIR>/configure -prefix ${EXTPREFIX_qt} -opensource -confirm-license -verbose -nomake examples -nomake tests -nomake tools -skip qt3d -skip qtactiveqt -skip qtcanvas3d -skip qtconnectivity -skip qtlocation -skip qtwayland -skip qtwebchannel -skip qtwebengine -skip qtwebsockets -skip qtwebview -skip qtserialport -skip qtdatavis3d -skip qtvirtualkeyboard -skip qtspeech -skip qtsensors -skip qtgamepad -skip qtscxml -skip qtremoteobjects -skip qtxmlpatterns -skip qtnetworkauth  -skip qtcharts -skip qtdatavis3d -skip qtgamepad -skip qtpurchasing -skip qtscxml -skip qtserialbus -skip qtspeech -skip qtvirtualkeyboard -android-sdk ${ANDROID_SDK_ROOT} -android-ndk ${CMAKE_ANDROID_NDK} -android-arch ${ANDROID_ABI} -xplatform android-clang -android-ndk-platform android-23 -make libs -qt-sqlite -openssl-linked OPENSSL_PREFIX=${EXTPREFIX_qt} -translationdir ${LOCALE_INSTALL_DIR} -system-zlib

      INSTALL_DIR ${EXTPREFIX_qt}

      BUILD_COMMAND $(MAKE)
      INSTALL_COMMAND $(MAKE) install
      UPDATE_COMMAND ""
      BUILD_IN_SOURCE 1

      DEPENDS ext_openssl ${ZLIB_DEP}
)
elseif (NOT APPLE)
  ExternalProject_Add(
      ext_qt
      DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
      URL http://download.qt.io/archive/qt/5.12/5.12.12/single/qt-everywhere-src-5.12.12.tar.xz
      URL_MD5 88e7a1c9952bf5732751c794716c6a55

      PATCH_COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0012-Synthesize-Enter-LeaveEvent-for-accepted-QTabletEven.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0013-Poison-Qt-s-headers-with-a-mark-about-presence-of-En.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0081-Fix-no-warning-for-overwriting-files-in-non-native-d.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0082-Make-jp-e-g-default-extensions-context-aware.patch
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

            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Do-not-crash-if-the-foundry-name-is-an-empty-space.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/bug449118-QMdiSubWindow-Set-correct-ControlLabel-size.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/bug449779-Check-for-uninitialized-QDockAreaLayoutInfo.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtdeclarative -i ${CMAKE_CURRENT_SOURCE_DIR}/Make-QQuickWindow-renderer-follow-real-logical-size.patch

            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/frac_dpi_paint_artifacts/0001-Add-faster-path-for-scaling-QRegion-with-multiple-re.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/frac_dpi_paint_artifacts/0002-Fix-QPainter-off-by-one-clipping-for-some-non-intege.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/frac_dpi_paint_artifacts/0003-Fix-under-non-integer-scaling-incorrect-update-area.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-QMenu-make-less-sensitive-to-mouse-jitter.patch


      CMAKE_ARGS -DOPENSSL_LIBS='-L${EXTPREFIX_qt}/lib -lssl -lcrypto'

      CONFIGURE_COMMAND <SOURCE_DIR>/configure -prefix ${EXTPREFIX_qt} -opensource -confirm-license -feature-relocatable -openssl-linked -verbose -nomake examples -skip qt3d -skip qtactiveqt -skip qtcanvas3d -skip qtconnectivity -skip qtlocation -skip qtwayland -skip qtwebchannel -skip qtwebengine -skip qtwebsockets -skip qtwebview -skip qtandroidextras -skip qtserialport -skip qtdatavis3d -skip qtvirtualkeyboard -skip qtspeech -skip qtsensors -skip qtgamepad -skip qtscxml -skip qtremoteobjects -skip qtxmlpatterns -skip qtnetworkauth  -skip qtcharts -skip qtdatavis3d -skip qtgamepad -skip qtpurchasing -skip qtscxml -skip qtserialbus -skip qtspeech -skip qtvirtualkeyboard -qt-sqlite -system-zlib

      INSTALL_DIR ${EXTPREFIX_qt}

      BUILD_COMMAND $(MAKE)
      INSTALL_COMMAND $(MAKE) install
      UPDATE_COMMAND ""
      BUILD_IN_SOURCE 1
      DEPENDS ${ZLIB_DEP}
  )
else( APPLE )
      # Qt is big - try and parallelize if at all possible
      include(ProcessorCount)
      ProcessorCount(NUM_CORES)
      if(NOT NUM_CORES EQUAL 0)
        if (NUM_CORES GREATER 2)
          # be nice...
          MATH( EXPR NUM_CORES "${NUM_CORES} - 2" )
        endif()
        set(PARALLEL_MAKE "make;-j${NUM_CORES}")
        message(STATUS "${EXTPREFIX_qt}:Parallelized make: ${PARALLEL_MAKE}")
      else()
        set(PARALLEL_MAKE "make")
      endif()

      if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "arm64")
        set(QT_DEVICE_OPTION "-device-option" "QMAKE_APPLE_DEVICE_ARCHS=arm64")
      endif()


      ExternalProject_Add(
        ext_qt
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL http://download.qt.io/official_releases/qt/5.12/5.12.12/single/qt-everywhere-src-5.12.12.tar.xz
        URL_MD5 88e7a1c9952bf5732751c794716c6a55

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0012-Synthesize-Enter-LeaveEvent-for-accepted-QTabletEven.patch
                COMMAND ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0013-Poison-Qt-s-headers-with-a-mark-about-presence-of-En.patch 
                COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0081-Fix-no-warning-for-overwriting-files-in-non-native-d.patch 
                COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0082-Make-jp-e-g-default-extensions-context-aware.patch
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

        LOG_DOWNLOAD        ON
        LOG_UPDATE          ON
        LOG_CONFIGURE       ON
        LOG_BUILD           ON
        LOG_TEST            ON
        LOG_INSTALL         ON
        BUILD_IN_SOURCE     ON

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
        BUILD_COMMAND ${PARALLEL_MAKE}
        INSTALL_COMMAND make install
        UPDATE_COMMAND ""
        BUILD_IN_SOURCE 1
        DEPENDS ${ZLIB_DEP}
      )
endif()
