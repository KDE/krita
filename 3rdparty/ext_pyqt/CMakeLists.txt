SET(PREFIX_ext_pyqt "${EXTPREFIX}" )
if (APPLE)
    SET(PYTHON_EXECUTABLE_PATH ${PREFIX_ext_pyqt}/bin/python3)
    if(NOT EXISTS ${PYTHON_EXECUTABLE_PATH})
      message("WARNING: using system python3!")
      SET(PYTHON_EXECUTABLE_PATH python3)
    endif()

    list(APPEND _PYQT_conf
        --confirm-license
        --qmake ${PREFIX_ext_pyqt}/bin/qmake
        --sip ${PREFIX_ext_pyqt}/bin/sip
        --sip-incdir ${PREFIX_ext_pyqt}/include
        --sipdir ${PREFIX_ext_pyqt}/share/sip
    )
    ExternalProject_Add( ext_pyqt
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://files.kde.org/krita/build/dependencies/PyQt5_gpl-5.13.1.tar.gz
        URL_MD5 c77af2f4e230b0053fd22bc2ece2e6c0
        
        CONFIGURE_COMMAND ${PYTHON_EXECUTABLE_PATH} <SOURCE_DIR>/configure.py ${_PYQT_conf}
        BUILD_COMMAND make

        # force single-threaded installation to avoid
        # race conditions in post-install .py script
        INSTALL_COMMAND make -j1 install
        
        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
    )
elseif(UNIX)
    SET(PYTHON_EXECUTABLE_PATH ${PREFIX_ext_pyqt}/bin/python3)
    if(NOT EXISTS ${PYTHON_EXECUTABLE_PATH})
      message("WARNING: using system python3!")
      SET(PYTHON_EXECUTABLE_PATH python3)
    endif()

    ExternalProject_Add(ext_pyqtbuilder
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://pypi.io/packages/source/P/PyQt-builder/PyQt-builder-1.10.0.tar.gz
        URL_HASH SHA256=86bd19fde83d92beaefacdeac1e26c6e1918c300ff78d7ec2a19973bf2cf21b5

        CONFIGURE_COMMAND ""

        BUILD_COMMAND ${PYTHON_EXECUTABLE_PATH} <SOURCE_DIR>/setup.py build ${_compiler} -j ${SUBMAKE_JOBS}

        INSTALL_COMMAND ${PYTHON_EXECUTABLE_PATH} <SOURCE_DIR>/setup.py install --skip-build --prefix=${PREFIX_ext_pyqt} --optimize=1

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
    )

    ExternalProject_Add( ext_pyqt5
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://pypi.python.org/packages/source/P/PyQt5/PyQt5-5.15.4.tar.gz
        URL_HASH SHA256=2a69597e0dd11caabe75fae133feca66387819fc9bc050f547e5551bce97e5be

        CONFIGURE_COMMAND sip-build --confirm-license --no-designer-plugin --no-qml-plugin --no-dbus-python --no-tools --no-make --jobs ${SUBMAKE_JOBS}

        BUILD_COMMAND ${CMAKE_COMMAND} -E chdir <SOURCE_DIR>/build make -j${SUBMAKE_JOBS}

        # force single-threaded installation to avoid
        # race conditions in post-install .py script
        INSTALL_COMMAND ${CMAKE_COMMAND} -E chdir <SOURCE_DIR>/build make -j1 install

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""

        DEPENDS ext_sip ext_pyqtbuilder
    )

    ExternalProject_Add( ext_pyqt5_sip
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://pypi.python.org/packages/source/P/PyQt5-sip/PyQt5_sip-12.9.0.tar.gz
        URL_HASH SHA256=d3e4489d7c2b0ece9d203ae66e573939f7f60d4d29e089c9f11daa17cfeaae32

        CONFIGURE_COMMAND ""

        BUILD_COMMAND ${PYTHON_EXECUTABLE_PATH} <SOURCE_DIR>/setup.py build ${_compiler} -j ${SUBMAKE_JOBS}

        # distutils already specifies the full path to the AppImage root
        # it trims the first path separator and appends the rest to the root below
        # if this is not set, it doesn't install the egg within site-packages
        INSTALL_COMMAND ${PYTHON_EXECUTABLE_PATH} <SOURCE_DIR>/setup.py install --root=/

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
    )

    add_custom_target(ext_pyqt)
    add_dependencies(ext_pyqt ext_pyqt5 ext_pyqt5_sip)
elseif(MINGW)

    list(APPEND _PYQT_conf
        --confirm-license
        --target-py-version 3.8
        --bindir ${PREFIX_ext_pyqt}/bin
        --qmake ${PREFIX_ext_pyqt}/bin/qmake.exe
        --sip ${PREFIX_ext_pyqt}/bin/sip.exe
        --sip-incdir ${PREFIX_ext_pyqt}/include
        --spec win32-g++
        --verbose
        --sipdir ${PREFIX_ext_pyqt}/share/sip
        --destdir ${PREFIX_ext_pyqt}/lib/krita-python-libs
        --stubsdir ${PREFIX_ext_pyqt}/lib/krita-python-libs/PyQt5
        --no-qml-plugin --no-python-dbus --no-qsci-api --no-tools
        --disable QtSql --disable QtTest --disable QtWinExtras --disable QtHelp
        --qmake ${PREFIX_ext_pyqt}/bin/qmake.exe
    )
    ExternalProject_Add( ext_pyqt
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://files.kde.org/krita/build/dependencies/PyQt5_gpl-5.13.1.zip
        URL_MD5 bf3c8d54d99a6ee355a309925e403c6e

        PATCH_COMMAND  ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/pyqt-configure-fix.patch

        CONFIGURE_COMMAND ${PYTHON_EXECUTABLE} <SOURCE_DIR>/configure.py ${_PYQT_conf}
        BUILD_COMMAND mingw32-make -j${SUBMAKE_JOBS} CXXFLAGS=-D_hypot=hypot LDFLAGS=${SECURITY_SHARED_LINKER_FLAGS}

        # force single-threaded installation to avoid
        # race conditions in post-install .py script
        INSTALL_COMMAND mingw32-make -j1 install
        
        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
    )

endif()

