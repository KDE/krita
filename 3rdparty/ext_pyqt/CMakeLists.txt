SET(PREFIX_ext_pyqt "${EXTPREFIX}" )

if (APPLE)
    SET(Python_EXECUTABLE ${PREFIX_ext_sip}/bin/python3)
    if(NOT EXISTS "${Python_EXECUTABLE}")
        message("WARNING: using system python3!")
        SET(Python_EXECUTABLE python3)
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

        CONFIGURE_COMMAND ${Python_EXECUTABLE} <SOURCE_DIR>/configure.py ${_PYQT_conf}
        BUILD_COMMAND make

        # force single-threaded installation to avoid
        # race conditions in post-install .py script
        INSTALL_COMMAND make -j1 install

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
    )

elseif(UNIX)
    SET(Python_EXECUTABLE ${PREFIX_ext_pyqt}/bin/python3)
    if(NOT EXISTS ${Python_EXECUTABLE})
      message("WARNING: using system python3!")
      SET(Python_EXECUTABLE python3)
    endif()

    ExternalProject_Add(ext_pyqtbuilder
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://pypi.io/packages/source/P/PyQt-builder/PyQt-builder-1.10.0.tar.gz
        URL_HASH SHA256=86bd19fde83d92beaefacdeac1e26c6e1918c300ff78d7ec2a19973bf2cf21b5

        CONFIGURE_COMMAND ""

        BUILD_COMMAND ${CMAKE_COMMAND} -E env 
            "PYTHONPATH=${KRITA_PYTHONPATH}"
            ${PYTHON_EXECUTABLE_PATH} <SOURCE_DIR>/setup.py build ${_compiler} -j ${SUBMAKE_JOBS}


        INSTALL_COMMAND ${CMAKE_COMMAND} -E env 
            "PYTHONPATH=${KRITA_PYTHONPATH}"
            ${PYTHON_EXECUTABLE_PATH} <SOURCE_DIR>/setup.py install --skip-build --prefix=${PREFIX_ext_pyqt} --optimize=1

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
    )

    ExternalProject_Add( ext_pyqt5
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://pypi.python.org/packages/source/P/PyQt5/PyQt5-5.15.4.tar.gz
        URL_HASH SHA256=2a69597e0dd11caabe75fae133feca66387819fc9bc050f547e5551bce97e5be

        CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env 
            "PYTHONPATH=${KRITA_PYTHONPATH}"
            ${PREFIX_ext_pyqt}/bin/sip-build --confirm-license --no-designer-plugin --no-qml-plugin --no-dbus-python --no-tools --no-make --jobs ${SUBMAKE_JOBS}

        BUILD_COMMAND ${CMAKE_COMMAND} -E env 
            "PYTHONPATH=${KRITA_PYTHONPATH}"
            ${CMAKE_COMMAND} -E chdir <SOURCE_DIR>/build
                make -j${SUBMAKE_JOBS}

        # force single-threaded installation to avoid
        # race conditions in post-install .py script
        INSTALL_COMMAND ${CMAKE_COMMAND} -E env 
            "PYTHONPATH=${KRITA_PYTHONPATH}"
            ${CMAKE_COMMAND} -E chdir <SOURCE_DIR>/build
                make -j1 install

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""

        DEPENDS ext_sip ext_pyqtbuilder
    )

    ExternalProject_Add( ext_pyqt5_sip
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://pypi.python.org/packages/source/P/PyQt5-sip/PyQt5_sip-12.9.0.tar.gz
        URL_HASH SHA256=d3e4489d7c2b0ece9d203ae66e573939f7f60d4d29e089c9f11daa17cfeaae32

        CONFIGURE_COMMAND ""

        BUILD_COMMAND ${CMAKE_COMMAND} -E env 
            "PYTHONPATH=${KRITA_PYTHONPATH}"
            ${PYTHON_EXECUTABLE_PATH} <SOURCE_DIR>/setup.py build ${_compiler} -j ${SUBMAKE_JOBS}

        # Here we need a flat-egg installation, otherwise the egg is deployed
        # inside its own directory in site-packages.
        # So we set as root our 3rdparty layout, and a sufficiently dud prefix.
        # See https://github.com/msys2/MINGW-packages/blob/8a4066591cf1e023971526c55bf2d5fe0c7aa84a/mingw-w64-pyqt5-sip/PKGBUILD
        INSTALL_COMMAND ${CMAKE_COMMAND} -E env 
            "PYTHONPATH=${KRITA_PYTHONPATH}"
            ${PYTHON_EXECUTABLE_PATH} <SOURCE_DIR>/setup.py install --prefix "." --root=${PREFIX_ext_pyqt} --optimize=1

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
    )

    add_custom_target(ext_pyqt)
    add_dependencies(ext_pyqt ext_pyqt5 ext_pyqt5_sip)
elseif(WIN32)
    if(MINGW)
        set(_compiler -c mingw32)
        set(_make_command mingw32-make)
    elseif(MSVC)
        set(_compiler -c msvc)
        set(_make_command jom)
    endif()

    krita_to_native_path("${PREFIX_ext_pyqt}" _pyqt_prefix)
    krita_to_native_path("${KRITA_PYTHONPATH}" _krita_pythonpath)
    string(TOLOWER ${_krita_pythonpath} _krita_pythonpath)
    krita_to_native_environment_path_list("${_krita_pythonpath}" _krita_pythonpath)

    ExternalProject_Add(ext_pyqtbuilder
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://pypi.io/packages/source/P/PyQt-builder/PyQt-builder-1.10.0.tar.gz
        URL_HASH SHA256=86bd19fde83d92beaefacdeac1e26c6e1918c300ff78d7ec2a19973bf2cf21b5

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Fix-linking-to-Python3.8-on-Windows.patch

        CONFIGURE_COMMAND ""

        BUILD_COMMAND ${CMAKE_COMMAND} -E env 
            PYTHONPATH=${_krita_pythonpath}
            ${Python_EXECUTABLE} <SOURCE_DIR>/setup.py build ${_compiler} -j ${SUBMAKE_JOBS}

        # HACK: Create lib\site-packages before installing
        # See e.g. https://access.redhat.com/errata/RHBA-2020:4437
        INSTALL_COMMAND ${CMAKE_COMMAND} -E make_directory ${_pyqt_prefix}\\lib\\site-packages
            COMMAND ${CMAKE_COMMAND} -E env 
                PYTHONPATH=${_krita_pythonpath}
                ${Python_EXECUTABLE} <SOURCE_DIR>/setup.py install --skip-build --prefix=${_pyqt_prefix} --optimize=1 --install-scripts=${_pyqt_prefix}\\bin

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
    )

    ExternalProject_Add( ext_pyqt5
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://pypi.python.org/packages/source/P/PyQt5/PyQt5-5.15.4.tar.gz
        URL_HASH SHA256=2a69597e0dd11caabe75fae133feca66387819fc9bc050f547e5551bce97e5be

        CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env 
            PYTHONPATH=${_krita_pythonpath}
            ${PREFIX_ext_pyqt}/bin/sip-build --confirm-license --no-designer-plugin --no-qml-plugin --no-dbus-python --no-tools --no-make --target-dir ${PREFIX_ext_pyqt}\\lib\\site-packages --jobs ${SUBMAKE_JOBS}

        BUILD_COMMAND ${CMAKE_COMMAND} -E chdir <SOURCE_DIR>/build
            ${CMAKE_COMMAND} -E env 
            PYTHONPATH=${_krita_pythonpath}
            ${_make_command} -j${SUBMAKE_JOBS}

        # force single-threaded installation to avoid
        # race conditions in post-install .py script
        INSTALL_COMMAND ${CMAKE_COMMAND} -E chdir <SOURCE_DIR>/build
            ${CMAKE_COMMAND} -E env 
            PYTHONPATH=${_krita_pythonpath}
            ${_make_command} -j1 install

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""

        DEPENDS ext_sip ext_pyqtbuilder
    )

    ExternalProject_Add( ext_pyqt5_sip
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://pypi.python.org/packages/source/P/PyQt5-sip/PyQt5_sip-12.9.0.tar.gz
        URL_HASH SHA256=d3e4489d7c2b0ece9d203ae66e573939f7f60d4d29e089c9f11daa17cfeaae32

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-MinGW-Distutils-support-hack.patch

        CONFIGURE_COMMAND ""

        BUILD_COMMAND ${CMAKE_COMMAND} -E env 
            PYTHONPATH=${_krita_pythonpath}
            ${Python_EXECUTABLE} <SOURCE_DIR>/setup.py build ${_compiler} -j ${SUBMAKE_JOBS}

        # Here we need a flat-egg installation, otherwise the egg is deployed
        # inside its own directory in site-packages.
        # So we set as root our 3rdparty layout, and a sufficiently dud prefix.
        # See https://github.com/msys2/MINGW-packages/blob/8a4066591cf1e023971526c55bf2d5fe0c7aa84a/mingw-w64-pyqt5-sip/PKGBUILD
        INSTALL_COMMAND ${CMAKE_COMMAND} -E env 
            PYTHONPATH=${_krita_pythonpath}
            ${Python_EXECUTABLE} <SOURCE_DIR>/setup.py install --skip-build --prefix "." --root=${_pyqt_prefix} --optimize=1 --install-scripts=${_pyqt_prefix}\\bin

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
    )

    add_custom_target(ext_pyqt)
    add_dependencies(ext_pyqt ext_pyqt5 ext_pyqt5_sip)
endif()

