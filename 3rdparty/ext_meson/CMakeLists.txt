SET(PREFIX_ext_meson "${EXTPREFIX}" )
if (WIN32)
    krita_to_native_path("${PREFIX_ext_meson}" _meson_prefix)

    ExternalProject_Add( ext_meson
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/mesonbuild/meson/releases/download/0.63.0rc1/meson-0.63.0rc1.tar.gz
        URL_HASH SHA256=4980031c700bd3dfbdda1b69ed6d568b2312690a26998ea145be3b36451df8bc

        CONFIGURE_COMMAND ""

        BUILD_COMMAND ${CMAKE_COMMAND} -E env 
            PYTHONPATH=${_krita_pythonpath}
            ${Python_EXECUTABLE} <SOURCE_DIR>/setup.py build -j ${SUBMAKE_JOBS}

        INSTALL_COMMAND ${CMAKE_COMMAND} -E env 
            PYTHONPATH=${_krita_pythonpath}
            ${Python_EXECUTABLE} <SOURCE_DIR>/setup.py install --skip-build --prefix=${_meson_prefix} --optimize=1  --install-scripts=${_meson_prefix}\\bin

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""

        DEPENDS ext_python ${NINJA_DEP}
    )
elseif(NOT CMAKE_CROSSCOMPILING)
    set(_meson_prefix "${PREFIX_ext_meson}")

    ExternalProject_Add( ext_meson
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/mesonbuild/meson/releases/download/0.63.0rc1/meson-0.63.0rc1.tar.gz
        URL_HASH SHA256=4980031c700bd3dfbdda1b69ed6d568b2312690a26998ea145be3b36451df8bc

        CONFIGURE_COMMAND chmod u+x <SOURCE_DIR>/setup.py

        BUILD_COMMAND ${CMAKE_COMMAND} -E env 
            PYTHONPATH=${_krita_pythonpath}
            SETUPTOOLS_USE_DISTUTILS=local
            ${Python_EXECUTABLE} <SOURCE_DIR>/setup.py build -j ${SUBMAKE_JOBS}

        INSTALL_COMMAND ${CMAKE_COMMAND} -E env 
            PYTHONPATH=${_krita_pythonpath}
            SETUPTOOLS_USE_DISTUTILS=local
            ${Python_EXECUTABLE} <SOURCE_DIR>/setup.py install --skip-build --prefix=${_meson_prefix} --optimize=1 --install-scripts=${PREFIX_ext_meson}/bin

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""

        DEPENDS ext_python ${NINJA_DEP}
    )
else()
    set(_meson_prefix "${PREFIX_ext_meson}")
    # Meson needs to be brought manually when crosscompiling.
    ExternalProject_Add(ext_meson
        DOWNLOAD_COMMAND ""

        CONFIGURE_COMMAND ${CMAKE_COMMAND} -E echo "Installing meson natively"
        BUILD_COMMAND ${CMAKE_COMMAND} -E env 
            PYTHONUSERBASE=${_meson_prefix}
            ${Python_EXECUTABLE} -m pip install --upgrade meson==0.63.0 --user --ignore-installed --verbose
        INSTALL_COMMAND ${CMAKE_COMMAND} -E env 
            PYTHONPATH=${_krita_pythonpath}
            ${Python_EXECUTABLE} -m pip show -f meson

        UPDATE_COMMAND ""

        DEPENDS ${NINJA_DEP}
    )
endif()
