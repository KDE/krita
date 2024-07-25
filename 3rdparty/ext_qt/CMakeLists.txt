SET(EXTPREFIX_qt  "${EXTPREFIX}")

set(QT_GIT_TAG "6d099d35da4958861ea62e2f54271a1dd8ea7707")

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
        -no-libproxy -no-system-proxies -icu -no-mtdev
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
        list(APPEND _QT_conf -sanitize address)
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

    if (NOT PERL_FOUND) # Implies strawberry perl
      list(APPEND QT_EXTRA_DEPS ext_strawberryperl)
    endif()


  ExternalProject_Add(
      ext_qt
      DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
      GIT_REPOSITORY https://invent.kde.org/szaman/qt5.git
      GIT_TAG ${QT_GIT_TAG}
      GIT_SUBMODULES qtbase qtdeclarative qtimageformats qttools qtsvg qtwinextras qttranslations
      GIT_SUBMODULES_RECURSE false
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
      DEPENDS ext_googleangle ${OPENSSL_DEP} ${QT_EXTRA_DEPS} ${ZLIB_DEP} ext_icu
  )
elseif (ANDROID)
  ExternalProject_Add(
      ext_qt
      DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
      GIT_REPOSITORY https://invent.kde.org/szaman/qt5.git
      GIT_TAG ${QT_GIT_TAG}
      GIT_SUBMODULES qtbase qtdeclarative qtimageformats qttools qtsvg qttranslations qtandroidextras
      GIT_SUBMODULES_RECURSE false
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
          -icu
          -I ${EXTPREFIX_qt}/include
          -L ${EXTPREFIX_qt}/lib

      BUILD_COMMAND ${QT_BUILD_COMMAND}
      INSTALL_COMMAND ${QT_INSTALL_COMMAND}

      INSTALL_DIR ${EXTPREFIX_qt}

      UPDATE_COMMAND ""
      BUILD_IN_SOURCE 0

      DEPENDS ext_openssl ${ZLIB_DEP} ext_icu
)
elseif (NOT APPLE)
    if (QT_ENABLE_DEBUG_INFO)
        # Set the option to build Qt with debugging info enabled
        set(QT_DEBUG_OPTIONS -force-debug-info)
    endif()
    if (QT_ENABLE_ASAN)
        list(APPEND _QT_conf -sanitize address)
    endif ()


    ExternalProject_Add(
      ext_qt
      DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
      GIT_REPOSITORY https://invent.kde.org/szaman/qt5.git
      GIT_TAG ${QT_GIT_TAG}
      GIT_SUBMODULES qtbase qtdeclarative qtimageformats qttools qtsvg qtx11extras qttranslations
      GIT_SUBMODULES_RECURSE false
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

    set(MACOS_QT_CONFIGURE_OPS
        -skip qt3d -skip qtactiveqt -skip qtcanvas3d -skip qtconnectivity -skip qtdoc -skip qtlocation -skip qtsensors -skip qtserialport -skip qtwayland
        -skip qtwebchannel -skip qtwebsockets -skip qtwebview -skip qtwebengine -skip qtxmlpatterns
        -skip qtcharts -skip qtdatavis3d  -skip qtgamepad -skip qtnetworkauth
        -skip qtpurchasing -skip qtremoteobjects -skip qtscxml -skip qtserialbus -skip qtspeech -skip qtvirtualkeyboard -nomake examples -nomake tools -no-compile-examples
        -no-dbus -no-iconv -no-qml-debug -no-libproxy -no-system-proxies -icu -no-mtdev -system-zlib -qt-pcre -qt-sqlite
        -opensource -confirm-license -openssl-linked -prefix ${EXTPREFIX_qt}
    )

    kis_ExternalProject_Add_macos(
      ext_qt

      GIT_REPOSITORY https://invent.kde.org/szaman/qt5.git
      GIT_TAG ${QT_GIT_TAG}
      GIT_SUBMODULES qtbase qtdeclarative qtimageformats qttools qtsvg qtmacextras qttranslations
      GIT_SUBMODULES_RECURSE false
      GIT_PROGRESS true
      GIT_REMOTE_UPDATE_STRATEGY REBASE

      WORKING_DIRECTORY <BINARY_DIR>-@ARCH@

      CONFIGURE_COMMAND -e "OPENSSL_LIBS=-L${EXTPREFIX_qt}/lib -lssl -lcrypto"
          <SOURCE_DIR>/configure
          ${MACOS_QT_CONFIGURE_OPS}
          -device-option QMAKE_APPLE_DEVICE_ARCHS=@ARCH@

      BUILD_COMMAND ${QT_BUILD_COMMAND}

      DEPENDS ${ZLIB_DEP} ext_icu ext_openssl
    )

    # Qt needs separate builds for universal compilation before qt6
    foreach(ARCH ${CMAKE_OSX_ARCHITECTURES})
        mkdir_build_arch_dir(${ARCH})
    endforeach()

endif()
