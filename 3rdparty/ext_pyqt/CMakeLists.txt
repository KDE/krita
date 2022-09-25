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
        URL https://pypi.io/packages/source/P/PyQt-builder/PyQt-builder-1.13.0.tar.gz
        URL_HASH SHA256=4877580c38ceb5320e129b381d083b0a8601c68166d8b99707f08fa0a1689eef

        CONFIGURE_COMMAND ""

        BUILD_COMMAND ${CMAKE_COMMAND} -E env 
            "PYTHONPATH=${KRITA_PYTHONPATH}"
            ${Python_EXECUTABLE} <SOURCE_DIR>/setup.py build ${_compiler} -j ${SUBMAKE_JOBS}

        INSTALL_COMMAND ${CMAKE_COMMAND} -E env 
            "PYTHONPATH=${KRITA_PYTHONPATH}"
            ${Python_EXECUTABLE} <SOURCE_DIR>/setup.py install --skip-build --prefix=${PREFIX_ext_pyqt} --optimize=1

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
    )

    ExternalProject_Add( ext_pyqt5
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://pypi.python.org/packages/source/P/PyQt5/PyQt5-5.15.7.tar.gz
        URL_HASH SHA256=755121a52b3a08cb07275c10ebb96576d36e320e572591db16cfdbc558101594

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
        URL https://pypi.python.org/packages/source/P/PyQt5-sip/PyQt5_sip-12.11.0.tar.gz
        URL_HASH SHA256=b4710fd85b57edef716cc55fae45bfd5bfac6fc7ba91036f1dcc3f331ca0eb39

        CONFIGURE_COMMAND ""

        BUILD_COMMAND ${CMAKE_COMMAND} -E env 
            "PYTHONPATH=${KRITA_PYTHONPATH}"
            ${Python_EXECUTABLE} <SOURCE_DIR>/setup.py build ${_compiler} -j ${SUBMAKE_JOBS}

        # Here we need a flat-egg installation, otherwise the egg is deployed
        # inside its own directory in site-packages.
        # So we set as root our 3rdparty layout, and a sufficiently dud prefix.
        # See https://github.com/msys2/MINGW-packages/blob/8a4066591cf1e023971526c55bf2d5fe0c7aa84a/mingw-w64-pyqt5-sip/PKGBUILD
        INSTALL_COMMAND ${CMAKE_COMMAND} -E env 
            "PYTHONPATH=${KRITA_PYTHONPATH}"
            ${Python_EXECUTABLE} <SOURCE_DIR>/setup.py install --prefix "." --root=${PREFIX_ext_pyqt} --optimize=1

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
        URL https://pypi.io/packages/source/P/PyQt-builder/PyQt-builder-1.13.0.tar.gz
        URL_HASH SHA256=4877580c38ceb5320e129b381d083b0a8601c68166d8b99707f08fa0a1689eef

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Fix-linking-to-Python3.8-on-Windows.patch
            COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0002-Add-additional-flavor-of-MinGW.patch

        CONFIGURE_COMMAND ""

        BUILD_COMMAND ${CMAKE_COMMAND} -E env 
            PYTHONPATH=${_krita_pythonpath}
            SETUPTOOLS_USE_DISTUTILS=local
            ${Python_EXECUTABLE} <SOURCE_DIR>/setup.py build ${_compiler} -j ${SUBMAKE_JOBS}

        # HACK: Create lib\site-packages before installing
        # See e.g. https://access.redhat.com/errata/RHBA-2020:4437
        INSTALL_COMMAND ${CMAKE_COMMAND} -E make_directory ${_pyqt_prefix}\\lib\\site-packages
            COMMAND ${CMAKE_COMMAND} -E env 
                PYTHONPATH=${_krita_pythonpath}
                SETUPTOOLS_USE_DISTUTILS=local
                ${Python_EXECUTABLE} <SOURCE_DIR>/setup.py install --skip-build --prefix=${_pyqt_prefix} --optimize=1 --install-scripts=${_pyqt_prefix}\\bin

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
    )

    ExternalProject_Add( ext_pyqt5
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://pypi.python.org/packages/source/P/PyQt5/PyQt5-5.15.7.tar.gz
        URL_HASH SHA256=755121a52b3a08cb07275c10ebb96576d36e320e572591db16cfdbc558101594

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
            ${_make_command} install

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""

        DEPENDS ext_sip ext_pyqtbuilder
    )

    ExternalProject_Add( ext_pyqt5_sip
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://pypi.python.org/packages/source/P/PyQt5-sip/PyQt5_sip-12.11.0.tar.gz
        URL_HASH SHA256=b4710fd85b57edef716cc55fae45bfd5bfac6fc7ba91036f1dcc3f331ca0eb39

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-MinGW-Distutils-support-hack.patch

        CONFIGURE_COMMAND ""

        BUILD_COMMAND ${CMAKE_COMMAND} -E env 
            PYTHONPATH=${_krita_pythonpath}
            SETUPTOOLS_USE_DISTUTILS=local
            ${Python_EXECUTABLE} <SOURCE_DIR>/setup.py build ${_compiler} -j ${SUBMAKE_JOBS}

        # Here we need a flat-egg installation, otherwise the egg is deployed
        # inside its own directory in site-packages.
        # So we set as root our 3rdparty layout, and a sufficiently dud prefix.
        # See https://github.com/msys2/MINGW-packages/blob/8a4066591cf1e023971526c55bf2d5fe0c7aa84a/mingw-w64-pyqt5-sip/PKGBUILD
        INSTALL_COMMAND ${CMAKE_COMMAND} -E env 
            PYTHONPATH=${_krita_pythonpath}
            SETUPTOOLS_USE_DISTUTILS=local
            ${Python_EXECUTABLE} <SOURCE_DIR>/setup.py install --skip-build --prefix "." --root=${_pyqt_prefix} --optimize=1 --install-scripts=${_pyqt_prefix}\\bin

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
    )

    add_custom_target(ext_pyqt)
    add_dependencies(ext_pyqt ext_pyqt5 ext_pyqt5_sip)
endif()

