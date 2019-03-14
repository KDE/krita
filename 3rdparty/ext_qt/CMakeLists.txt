SET(EXTPREFIX_qt  "${EXTPREFIX}")
if (WIN32)
    list(APPEND _QT_conf -skip qt3d -skip qtactiveqt -skip qtcanvas3d
        -skip qtconnectivity -skip qtdoc -skip qtgraphicaleffects
        -skip qtlocation -skip qtsensors -skip qtserialport -skip qtwayland
        -skip qtwebchannel -skip qtwebengine -skip qtwebsockets -skip qtwebview
        -skip qtxmlpatterns -no-sql-sqlite -nomake examples -nomake tools
        -no-compile-examples -no-dbus -no-iconv -no-qml-debug -no-ssl
        -no-libproxy -no-system-proxies -no-icu -no-mtdev
        -skip qtcharts -skip qtdatavis3d -skip qtgamepad -skip qtnetworkauth
        -skip qtpurchasing -skip qtremoteobjects -skip qtscxml -skip qtserialbus
        -skip qtspeech -skip qtvirtualkeyboard
        #
        -qt-zlib -qt-pcre -qt-libpng -qt-libjpeg
        #
        -opensource -confirm-license
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

  ExternalProject_Add(
      ext_qt
      DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
      URL https://download.qt.io/archive/qt/5.12/5.12.1/single/qt-everywhere-src-5.12.1.tar.xz
      URL_HASH SHA1=018b7467faeda9979fe6b4a435214903c3a20018
      PATCH_COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-disable-wintab.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0002-Don-t-request-the-MIME-image-every-time-Windows-asks.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0003-Hack-always-return-we-support-DIBV5.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0004-Fix-debug-on-openGL-ES.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0005-cumulative-patch-for-hdr.patch

      INSTALL_DIR ${EXTPREFIX_qt}
      CONFIGURE_COMMAND <SOURCE_DIR>/configure.bat ${_QT_conf}

      BUILD_COMMAND mingw32-make -j${SUBMAKE_JOBS}
      INSTALL_COMMAND mingw32-make -j${SUBMAKE_JOBS} install
      UPDATE_COMMAND ""
      # Use a short name to reduce the chance of exceeding path length limit
      SOURCE_DIR s
      BINARY_DIR b
      DEPENDS ext_patch
  )
elseif (NOT APPLE)
  ExternalProject_Add(
      ext_qt
      DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
      URL https://download.qt.io/archive/qt/5.12/5.12.1/single/qt-everywhere-src-5.12.1.tar.xz
      URL_HASH SHA1=018b7467faeda9979fe6b4a435214903c3a20018
      URL_MD5 c5e275ab0ed7ee61d0f4b82cd471770d

      PATCH_COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0010-Fix-tablet-jitter-on-X11.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0011-Add-an-ID-for-recognition-of-UGEE-tablets.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0012-Synthesize-Enter-LeaveEvent-when-QTabletEvent-is-acc.patch
            COMMAND  ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0013-Poison-Qt-s-headers-with-a-mark-about-presence-of-En.patch

      CMAKE_ARGS -DOPENSSL_LIBS='-L${EXTPREFIX_qt}/lib -lssl -lcrypto'

      CONFIGURE_COMMAND <SOURCE_DIR>/configure -prefix ${EXTPREFIX_qt} -opensource -confirm-license -openssl-linked -verbose -nomake examples -skip qt3d -skip qtactiveqt -skip qtcanvas3d -skip qtconnectivity -skip qtgraphicaleffects -skip qtlocation -skip qtwayland -skip qtwebchannel -skip qtwebengine -skip qtwebsockets -skip qtwebview -skip qtandroidextras -skip qtserialport -skip qtdatavis3d -skip qtvirtualkeyboard -skip qtspeech -skip qtsensors -skip qtgamepad -skip qtscxml -skip qtremoteobjects -skip qtxmlpatterns -skip qtnetworkauth  -skip qtcharts -skip qtdatavis3d -skip qtgamepad -skip qtpurchasing -skip qtscxml -skip qtserialbus -skip qtspeech -skip qtvirtualkeyboard -skip qtmultimedia
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
        string(REGEX MATCH "Xcode ([0-9]([.][0-9])+)" version_match ${xcodebuild_version})
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
        set(ext_qt_PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/macdeploy-qt.diff
                COMMAND ${PATCH_COMMAND} -p1 -b -d <SOURCE_DIR>/qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/mac_standardpaths_qtbug-61159.diff
                COMMAND ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0012-Synthesize-Enter-LeaveEvent-when-QTabletEvent-is-acc.patch
                COMMAND ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0013-Poison-Qt-s-headers-with-a-mark-about-presence-of-En.patch

                #COMMAND ${PATCH_COMMAND} -p1 -b -d <SOURCE_DIR>/qtbase/mkspecs/features/mac -i ${CMAKE_CURRENT_SOURCE_DIR}/mac-default.patch
            )
        message(STATUS "${EXTPREFIX_qt}:Additional patches injected.")
      else()
        # No extra patches will be applied
        # NOTE: defaults for some untested scenarios like xcrun fails and xcode_version < 8.
        # NOTE: that is uncharted territory and (hopefully) a very unlikely scenario...
        set(ext_qt_PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/macdeploy-qt.diff
                         COMMAND ${PATCH_COMMAND} -p1 -d qtbase -i ${CMAKE_CURRENT_SOURCE_DIR}/0012-Synthesize-Enter-LeaveEvent-when-QTabletEvent-is-acc.patch
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


      ExternalProject_Add(ext_qt
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

        LOG_DOWNLOAD        ON
        LOG_UPDATE          ON
        LOG_CONFIGURE       ON
        LOG_BUILD           ON
        LOG_TEST            ON
        LOG_INSTALL         ON
        BUILD_IN_SOURCE     ON

       URL https://download.qt.io/archive/qt/5.12/5.12.1/single/qt-everywhere-src-5.12.1.tar.xz
       URL_HASH MD5=6a37466c8c40e87d4a19c3f286ec2542

        CMAKE_ARGS -DOPENSSL_LIBS='-L${EXTPREFIX_qt}/lib -lssl -lcrypto'

        INSTALL_DIR ${EXTPREFIX_qt}
        CONFIGURE_COMMAND <SOURCE_DIR>/configure
        -skip qt3d -skip qtactiveqt -skip qtcanvas3d -skip qtconnectivity -skip qtdoc -skip qtgraphicaleffects -skip qtlocation -skip qtsensors -skip qtserialport -skip qtwayland
            -skip qtwebchannel -skip qtwebsockets -skip qtwebview -skip qtxmlpatterns -no-sql-sqlite -skip qtcharts -skip qtdatavis3d  -skip qtgamepad -skip qtnetworkauth -skip qtpurchasing -skip qtremoteobjects -skip qtscxml -skip qtserialbus -skip qtspeech -skip qtvirtualkeyboard -nomake examples -nomake tools -no-compile-examples -no-dbus -no-iconv -no-qml-debug -no-libproxy -no-system-proxies -no-icu -no-mtdev -system-zlib -qt-pcre -opensource -confirm-license  -openssl-linked -prefix ${EXTPREFIX_qt}
        BUILD_COMMAND ${PARALLEL_MAKE}
        INSTALL_COMMAND make install
        UPDATE_COMMAND ""
        BUILD_IN_SOURCE 1
      )
endif()
