SET(EXTPREFIX_qt  "${EXTPREFIX}")
if (WIN32)
    list(APPEND _QT_conf -skip qt3d -skip qtactiveqt -skip qtcanvas3d
        -skip qtconnectivity -skip qtdoc -skip qtgraphicaleffects
        -skip qtlocation -skip qtsensors -skip qtserialport -skip qtwayland
        -skip qtwebchannel -skip qtwebengine -skip qtwebsockets -skip qtwebview
        -skip qtxmlpatterns -nomake examples -nomake tools
        -no-compile-examples -no-dbus -no-iconv -no-qml-debug 
        -no-libproxy -no-system-proxies -no-icu -no-mtdev
        -skip qtcharts -skip qtdatavis3d -skip qtgamepad -skip qtnetworkauth
        -skip qtpurchasing -skip qtremoteobjects -skip qtscxml -skip qtserialbus
        -skip qtspeech -skip qtvirtualkeyboard -qt-sqlite
        #
        -qt-zlib -qt-pcre -qt-libpng -qt-libjpeg -openssl-linked
        -I ${EXTPREFIX_qt}/include
        -L ${EXTPREFIX_qt}/lib
        #
        -opensource -confirm-license
        -feature-relocatable
        #
        -release -platform win32-g++ -prefix ${EXTPREFIX_qt}
        QMAKE_LFLAGS_APP+=${SECURITY_EXE_LINKER_FLAGS}
        QMAKE_LFLAGS_SHLIB+=${SECURITY_SHARED_LINKER_FLAGS}
        QMAKE_LFLAGS_SONAME+=${SECURITY_SHARED_LINKER_FLAGS}
    )
    if (QT_ENABLE_DEBUG_INFO)
        # Set the option to build Qt with debugging info enabled
        list(APPEND _QT_conf -force-debug-info)
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
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0061-Hack-to-hide-1px-border-with-OpenGL-fullscreen-hack.patch
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
        COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0116-Make-Qt-relocatable.diff
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
    )

  ExternalProject_Add(
      ext_qt
      DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
      URL http://download.qt.io/archive/qt/5.12/5.12.9/single/qt-everywhere-src-5.12.9.zip
      URL_MD5 872ca27e9038d8fdbefff51a9db3503b

      PATCH_COMMAND ${ext_qt_PATCH_COMMAND}

      INSTALL_DIR ${EXTPREFIX_qt}
      CONFIGURE_COMMAND <SOURCE_DIR>/configure.bat ${_QT_conf}

      BUILD_COMMAND mingw32-make -j${SUBMAKE_JOBS}

      INSTALL_COMMAND mingw32-make -j${SUBMAKE_JOBS} install
      UPDATE_COMMAND ""
      # Use a short name to reduce the chance of exceeding path length limit
      SOURCE_DIR s
      BINARY_DIR b
      DEPENDS ext_patch ext_openssl
  )
elseif (ANDROID)
  ExternalProject_Add(
      ext_qt
      DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
      URL http://download.qt.io/archive/qt/5.12/5.12.9/single/qt-everywhere-src-5.12.9.tar.xz
      URL_MD5 fa2646280cf38180689c29c393cddd05

      PATCH_COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0093-Fix-QLocale-system-and-uiLanguages-for-the-mobile-pl.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0094-Bugfix-fix-the-offset-bug-when-using-Stylus-with-And.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0095-Add-support-for-pen-tilt-rotation-for-Android.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0097-Pass-keyboard-modifiers-for-mouse-and-tablet-events.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0099-Emulate-Long-Press-as-Right-Click-on-Android.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0100-Fix-artifacts-when-rendering-multisubpath-dashed-QPa.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0111-Fix-unbalanced-KeyPress-Release-events-in-children-o.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0112-Fix-shortcuts-assigned-to-special-keys-on-non-latin-.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0113-Fix-multiple-shortcuts-match-from-different-keyboard.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0096-Android-Pass-the-button-state-through-JNI.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0101-Android-Handle-the-ACTION_CANCEL-Event-from-Android.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0102-Android-Properly-handle-Tablet-events.patch

            # Upstream Qt 5.13
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0040-Add-file-engine-for-Android-content-URLs.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0041-Android-handle-check-permissions-before-any-file-ope.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0042-Android-Add-support-for-getting-information-about-co.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0043-Also-fall-back-to-direct-write-for-Android-content-U.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0047-platform-android-Add-native-file-dialog.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0048-Fix-Text-select-handle-misplaced-on-fields-inside-QD.patch

            # workarounds, hacks and fixes
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0045-Android-find-column-index-at-runtime.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0046-Android-return-the-actual-file-name-the-URI-is-point.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0048-Android-Properly-close-the-File-Descriptor.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0049-Android-Workarounds-to-explicitly-handle-content-URI.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0050-Android-Add-a-way-to-check-permissions-for-external-.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0051-Android-Add-clipboard-buffer-for-mimeTypes-that-aren.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0052-Android-Move-events-handling-to-QtLayout.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0053-Android-Use-processOpenModeFlags-to-get-right-OpenMo.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0054-Android-use-EXTRA_INITIAL_URI-as-the-defaultDir.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0055-Android-prevent-out-of-order-events.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0056-Android-check-if-the-Uri-is-writable.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0056-Android-Give-preference-to-certain-WindowTypes.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0057-Android-Add-window-manager-for-raster-based-QWidgets.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Android-Don-t-invoke-Qt-s-exit-path.patch

            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0105-Android-Don-t-hide-Menubar-even-if-MenuKey-is-presen.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/106-Android-don-t-use-getExternalStorageDirectory-and-ge.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Remove-chunk-size-limits.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Check-if-we-reached-the-end-of-Iterator.patch

      CONFIGURE_COMMAND <SOURCE_DIR>/configure -prefix ${EXTPREFIX_qt} -opensource -confirm-license -verbose -nomake examples -nomake tests -nomake tools -skip qt3d -skip qtactiveqt -skip qtcanvas3d -skip qtconnectivity -skip qtgraphicaleffects -skip qtlocation -skip qtwayland -skip qtwebchannel -skip qtwebengine -skip qtwebsockets -skip qtwebview -skip qtserialport -skip qtdatavis3d -skip qtvirtualkeyboard -skip qtspeech -skip qtsensors -skip qtgamepad -skip qtscxml -skip qtremoteobjects -skip qtxmlpatterns -skip qtnetworkauth  -skip qtcharts -skip qtdatavis3d -skip qtgamepad -skip qtpurchasing -skip qtscxml -skip qtserialbus -skip qtspeech -skip qtvirtualkeyboard -android-sdk ${ANDROID_SDK_ROOT} -android-ndk ${CMAKE_ANDROID_NDK} -android-arch ${ANDROID_ABI} -xplatform android-clang -android-ndk-platform android-23 -make libs -qt-sqlite -openssl-linked OPENSSL_PREFIX=${EXTPREFIX_qt}

      INSTALL_DIR ${EXTPREFIX_qt}

      BUILD_COMMAND $(MAKE)
      INSTALL_COMMAND $(MAKE) install
      UPDATE_COMMAND ""
      BUILD_IN_SOURCE 1

      DEPENDS ext_openssl
)
elseif (NOT APPLE)
  ExternalProject_Add(
      ext_qt
      DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
      URL http://download.qt.io/archive/qt/5.12/5.12.9/single/qt-everywhere-src-5.12.9.tar.xz
      URL_MD5 fa2646280cf38180689c29c393cddd05

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
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0116-Make-Qt-relocatable.diff
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0119-Fix-relocated-Qt-to-be-found-when-its-path-has-non-A.patch


      CMAKE_ARGS -DOPENSSL_LIBS='-L${EXTPREFIX_qt}/lib -lssl -lcrypto'

      CONFIGURE_COMMAND <SOURCE_DIR>/configure -prefix ${EXTPREFIX_qt} -opensource -confirm-license -feature-relocatable -openssl-linked -verbose -nomake examples -skip qt3d -skip qtactiveqt -skip qtcanvas3d -skip qtconnectivity -skip qtgraphicaleffects -skip qtlocation -skip qtwayland -skip qtwebchannel -skip qtwebengine -skip qtwebsockets -skip qtwebview -skip qtandroidextras -skip qtserialport -skip qtdatavis3d -skip qtvirtualkeyboard -skip qtspeech -skip qtsensors -skip qtgamepad -skip qtscxml -skip qtremoteobjects -skip qtxmlpatterns -skip qtnetworkauth  -skip qtcharts -skip qtdatavis3d -skip qtgamepad -skip qtpurchasing -skip qtscxml -skip qtserialbus -skip qtspeech -skip qtvirtualkeyboard -qt-sqlite

      INSTALL_DIR ${EXTPREFIX_qt}

      BUILD_COMMAND $(MAKE)
      INSTALL_COMMAND $(MAKE) install
      UPDATE_COMMAND ""
      BUILD_IN_SOURCE 1
  )
else( APPLE )

      # XCODE_VERSION is set by CMake when using the Xcode generator, otherwise we need
      # to detect it manually here.
      if (NOT XCODE_VERSION)
        execute_process(
          COMMAND xcodebuild -version
          OUTPUT_VARIABLE xcodebuild_version
          OUTPUT_STRIP_TRAILING_WHITESPACE
          ERROR_FILE /dev/null
        )
        # string(REGEX MATCH "Xcode ([0-9]+([.][0-9]+)*)" version_match ${xcodebuild_version})
        string(REGEX MATCH "Xcode ([0-9]+([.][0-9]+)*)" version_match "${xcodebuild_version}")
        if (version_match)
          message(STATUS "${EXTPREFIX_qt}:Identified Xcode Version: ${CMAKE_MATCH_1}")
          set(XCODE_VERSION ${CMAKE_MATCH_1})
        else()
          # If detecting Xcode version failed, set a crazy high version so we default
          # to the newest.
          set(XCODE_VERSION 99)
          message(WARNING "${EXTPREFIX_qt}:Failed to detect the version of an installed copy of Xcode, falling back to highest supported version. Set XCODE_VERSION to override.")
        endif(version_match)
      endif(NOT XCODE_VERSION)

      # -------------------------------------------------------------------------------
      # Verify the Xcode installation on Mac OS like Qt5.7 does/will
      # If not stop now, the system isn't configured correctly for Qt.
      # No reason to even proceed.
      # -------------------------------------------------------------------------------
      set(XCSELECT_OUTPUT)
      find_program(XCSELECT_PROGRAM "xcode-select")
      if(XCSELECT_PROGRAM)
        message(STATUS "${EXTPREFIX_qt}:Found XCSELECT_PROGRAM as ${XCSELECT_PROGRAM}")
        set(XCSELECT_COMMAND ${XCSELECT_PROGRAM}
          "--print-path")
        execute_process(
          COMMAND ${XCSELECT_COMMAND}
          RESULT_VARIABLE XCSELECT_COMMAND_RESULT
          OUTPUT_VARIABLE XCSELECT_COMMAND_OUTPUT
          ERROR_FILE /dev/null
        )
        if(NOT XCSELECT_COMMAND_RESULT)
          # returned 0, we're ok.
          string(REGEX REPLACE
            "[ \t]*[\r\n]+[ \t]*" ";"
            XCSELECT_COMMAND_OUTPUT ${XCSELECT_COMMAND_OUTPUT})
        else()
          string(REPLACE ";" " " XCSELECT_COMMAND_STR "${XCSELECT_COMMAND}")
          # message(STATUS "${XCSELECT_COMMAND_STR}")
          message(FATAL_ERROR "${EXTPREFIX_qt}:${XCSELECT_PROGRAM} test failed with status ${XCSELECT_COMMAND_RESULT}")
        endif()
      else()
        message(FATAL_ERROR "${EXTPREFIX_qt}:${XCSELECT_PROGRAM} not found. No Xcode is selected. Use xcode-select -switch to choose an Xcode version")
      endif()

      # Belts and suspenders
      # Beyond all the Xcode and Qt version checking, the proof of the pudding
      # lies in the success/failure of this command: xcrun --find xcrun.
      # On failure a patch is necessary, otherwise we're ok
      # So hard check xcrun now...
      set(XCRUN_OUTPUT)
      find_program(XCRUN_PROGRAM "xcrun")
      if(XCRUN_PROGRAM)
        message(STATUS "${EXTPREFIX_qt}:Found XCRUN_PROGRAM as ${XCRUN_PROGRAM}")
        set(XCRUN_COMMAND ${XCRUN_PROGRAM}
          "--find xcrun")
        execute_process(
          COMMAND ${XCRUN_COMMAND}
          RESULT_VARIABLE XCRUN_COMMAND_RESULT
          OUTPUT_VARIABLE XCRUN_COMMAND_OUTPUT
          ERROR_FILE /dev/null
        )
        if(NOT XCRUN_COMMAND_RESULT)
          # returned 0, we're ok.
          string(REGEX REPLACE
            "[ \t]*[\r\n]+[ \t]*" ";"
            XCRUN_COMMAND_OUTPUT ${XCRUN_COMMAND_OUTPUT})
        else()
          string(REPLACE ";" " " XCRUN_COMMAND_STR "${XCRUN_COMMAND}")
          # message(STATUS "${XCRUN_COMMAND_STR}")
          message(STATUS "${EXTPREFIX_qt}:xcrun test failed with status ${XCRUN_COMMAND_RESULT}")
        endif()
      else()
        message(STATUS "${EXTPREFIX_qt}:xcrun not found -- ${XCRUN_PROGRAM}")
      endif()
      #
      # Now configure ext_qt accordingly
      #
      if ((XCRUN_COMMAND_RESULT) AND (NOT (XCODE_VERSION VERSION_LESS 8.0.0)))
        # Fix Xcode xcrun related issue.
        # NOTE: This should be fixed by Qt 5.7.1 see here: http://code.qt.io/cgit/qt/qtbase.git/commit/?h=dev&id=77a71c32c9d19b87f79b208929e71282e8d8b5d9
        # NOTE: but no one's holding their breath.
        set(ext_qt_PATCH_COMMAND ${PATCH_COMMAND}} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0012-Synthesize-Enter-LeaveEvent-for-accepted-QTabletEven.patch
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

                #COMMAND ${PATCH_COMMAND} -p1 -b -d <SOURCE_DIR>/qtbase/mkspecs/features/mac -i ${CMAKE_CURRENT_SOURCE_DIR}/mac-default.patch
            )
        message(STATUS "${EXTPREFIX_qt}:Additional patches injected.")
      else()
        # No extra patches will be applied
        # NOTE: defaults for some untested scenarios like xcrun fails and xcode_version < 8.
        # NOTE: that is uncharted territory and (hopefully) a very unlikely scenario...
        set(ext_qt_PATCH_COMMAND ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0012-Synthesize-Enter-LeaveEvent-for-accepted-QTabletEven.patch
                COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0081-Fix-no-warning-for-overwriting-files-in-non-native-d.patch
                COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0082-Make-jp-e-g-default-extensions-context-aware.patch
                COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0100-Fix-artifacts-when-rendering-multisubpath-dashed-QPa.patch
                COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0101-Don-t-eat-ShortcutOverride-events-when-there-is-a-pa.patch
                COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0110-Pass-proper-dirty-rect-in-paint-event-to-QOpenGLWidg.patch
                COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0111-Fix-unbalanced-KeyPress-Release-events-in-children-o.patch
                COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0112-Fix-shortcuts-assigned-to-special-keys-on-non-latin-.patch
                COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0113-Fix-multiple-shortcuts-match-from-different-keyboard.patch
                COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Check-if-we-reached-the-end-of-Iterator.patch
            )
      endif()

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

        LOG_DOWNLOAD        ON
        LOG_UPDATE          ON
        LOG_CONFIGURE       ON
        LOG_BUILD           ON
        LOG_TEST            ON
        LOG_INSTALL         ON
        BUILD_IN_SOURCE     ON

        URL http://download.qt.io/official_releases/qt/5.12/5.12.10/single/qt-everywhere-src-5.12.10.tar.xz
        URL_MD5 a781a0e247400e764c0730b8fb54226f

        CMAKE_ARGS -DOPENSSL_LIBS='-L${EXTPREFIX_qt}/lib -lssl -lcrypto'

        INSTALL_DIR ${EXTPREFIX_qt}
        CONFIGURE_COMMAND <SOURCE_DIR>/configure
        -skip qt3d -skip qtactiveqt -skip qtcanvas3d -skip qtconnectivity -skip qtdoc -skip qtgraphicaleffects -skip qtlocation -skip qtsensors -skip qtserialport -skip qtwayland
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
      )
endif()
