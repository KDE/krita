SET(PREFIX_ext_pyqt "${EXTPREFIX}" )

if(UNIX)
    if (QT_ENABLE_ASAN)
       set(_extra_pyqt_config --qmake-setting QMAKE_LFLAGS+=-fsanitize=address --qmake-setting QMAKE_CFLAGS+=-fsanitize=address --qmake-setting QMAKE_CXXFLAGS+=-fsanitize=address)
    elseif(APPLE)
        if(MACOS_ARCHS GREATER 1)
            set(_extra_pyqt_config --qmake-setting "QMAKE_APPLE_DEVICE_ARCHS=${CMAKE_OSX_ARCHITECTURES_STR}")
        endif()
    else ()
       unset(_extra_pyqt_config)
    endif ()

    ExternalProject_Add(ext_pyqtbuilder
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://pypi.io/packages/source/P/PyQt-builder/PyQt-builder-1.13.0.tar.gz
        URL_HASH SHA256=4877580c38ceb5320e129b381d083b0a8601c68166d8b99707f08fa0a1689eef

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-restrict-sip-version.patch

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
        URL https://pypi.python.org/packages/source/P/PyQt6/PyQt6-5.15.7.tar.gz
        URL_HASH SHA256=755121a52b3a08cb07275c10ebb96576d36e320e572591db16cfdbc558101594

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Fix-Qt-composite-module-lacking-a-platform-tag-on-ma.patch

        CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env 
            "PYTHONPATH=${KRITA_PYTHONPATH}"
            ${PREFIX_ext_pyqt}/bin/sip-build --confirm-license --no-designer-plugin --no-qml-plugin --no-dbus-python --no-tools --no-make --jobs ${SUBMAKE_JOBS} ${_extra_pyqt_config}

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
        URL https://pypi.python.org/packages/source/P/PyQt6-sip/PyQt6_sip-12.11.0.tar.gz
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

    if (QT_ENABLE_ASAN)
       set(_extra_pyqt_config --qmake-setting QMAKE_LFLAGS+=-fsanitize=address --qmake-setting QMAKE_CFLAGS+=-fsanitize=address --qmake-setting QMAKE_CXXFLAGS+=-fsanitize=address)
    else ()
       unset(_extra_pyqt_config)
    endif ()

    krita_to_native_path("${PREFIX_ext_pyqt}" _pyqt_prefix)
    krita_to_native_path("${KRITA_PYTHONPATH}" _krita_pythonpath)
    string(TOLOWER ${_krita_pythonpath} _krita_pythonpath)
    krita_to_native_environment_path_list("${_krita_pythonpath}" _krita_pythonpath)

    ExternalProject_Add(ext_pyqtbuilder
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://pypi.io/packages/source/P/PyQt-builder/PyQt-builder-1.13.0.tar.gz
        URL_HASH SHA256=4877580c38ceb5320e129b381d083b0a8601c68166d8b99707f08fa0a1689eef

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Support-MinGW-and-MSYS-Python-builds.patch
              COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Fix-building-pyqt-on-Windows.patch

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
        URL https://pypi.python.org/packages/source/P/PyQt6/PyQt6-5.15.7.tar.gz
        URL_HASH SHA256=755121a52b3a08cb07275c10ebb96576d36e320e572591db16cfdbc558101594

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Fix-Qt-composite-module-lacking-a-platform-tag-on-ma.patch

        CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env 
            PYTHONPATH=${_krita_pythonpath}
            ${PREFIX_ext_pyqt}/bin/sip-build --confirm-license --no-designer-plugin --no-qml-plugin --no-dbus-python --no-tools --no-make --target-dir ${PREFIX_ext_pyqt}\\lib\\site-packages --jobs ${SUBMAKE_JOBS} ${_extra_pyqt_config}

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
        URL https://pypi.python.org/packages/source/P/PyQt6-sip/PyQt6_sip-12.11.0.tar.gz
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

