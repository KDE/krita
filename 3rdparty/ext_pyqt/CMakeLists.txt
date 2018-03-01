SET(PREFIX_ext_pyqt "${EXTPREFIX}" )
if (UNIX)
    SET(PYTHON_EXECUTABLE_PATH ${PREFIX_ext_sip}/bin/python3)
    if(NOT EXISTS ${PYTHON_EXECUTABLE_PATH})
      message("WARNING: using system python3!")
      SET(PYTHON_EXECUTABLE_PATH python3)
    endif()
    ExternalProject_Add( ext_pyqt
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://files.kde.org/krita/build/dependencies/PyQt5_gpl-5.9.2.zip
        URL_MD5 67eacf2f486f77136c9758e03fe6889e
        
        CONFIGURE_COMMAND ${PYTHON_EXECUTABLE_PATH} <SOURCE_DIR>/configure.py --confirm-license --qmake ${PREFIX_ext_pyqt}/bin/qmake --sip ${PREFIX_ext_pyqt}/bin/sip --sip-incdir ${PREFIX_ext_pyqt}/include --sipdir ${PREFIX_ext_pyqt}/share/sip
        BUILD_COMMAND make
        INSTALL_COMMAND make install
        
        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
    )
elseif(MINGW)
    list(APPEND _PYQT_conf
        --confirm-license
        --target-py-version 3.6
        --bindir ${PREFIX_ext_pyqt}/bin
        --qt ${PREFIX_ext_pyqt}
        --sip ${PREFIX_ext_pyqt}/bin/sip.exe
        --sip-incdir ${PREFIX_ext_pyqt}/include
        --spec win32-g++
        --verbose
        --sipdir ${PREFIX_ext_pyqt}/share/sip
        --destdir ${PREFIX_ext_pyqt}/lib/krita-python-libs
        --stubsdir ${PREFIX_ext_pyqt}/lib/krita-python-libs/PyQt5
        --no-qml-plugin --no-python-dbus --no-qsci-api --no-tools
        --disable QtSql --disable QtTest --disable QtWinExtras
    )
    ExternalProject_Add( ext_pyqt
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://files.kde.org/krita/build/dependencies/PyQt5_gpl-5.9.2.zip
        URL_MD5 67eacf2f486f77136c9758e03fe6889e
        PATCH_COMMAND  ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/pyqt-configure-fix.patch

        CONFIGURE_COMMAND ${PYTHON_EXECUTABLE} <SOURCE_DIR>/configure.py ${_PYQT_conf}
        BUILD_COMMAND mingw32-make -j${SUBMAKE_JOBS} CXXFLAGS=-D_hypot=hypot LDFLAGS=${SECURITY_SHARED_LINKER_FLAGS}
        INSTALL_COMMAND mingw32-make -j${SUBMAKE_JOBS} install
        
        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
    )

endif()

