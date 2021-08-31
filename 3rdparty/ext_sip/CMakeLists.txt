SET(PREFIX_ext_sip "${EXTPREFIX}" )

if(APPLE)
    if(NOT Python_FOUND)
        SET(Python_EXECUTABLE ${PREFIX_ext_sip}/bin/python3)
        if(NOT EXISTS "${Python_EXECUTABLE}")
            message("WARNING: using system python3!")
            SET(Python_EXECUTABLE python3)
        endif()
    endif()

    ExternalProject_Add( ext_sip
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://files.kde.org/krita/build/dependencies/sip-4.19.19.tar.gz
        URL_MD5 98111479309dc472410f26080d6d4a88

        CMAKE_ARGS -DPYTHON_INCLUDE_DIR=${PREFIX_ext_sip}/lib/Python.framework/Headers
        CONFIGURE_COMMAND ${Python_EXECUTABLE} <SOURCE_DIR>/configure.py --deployment-target=10.11
            -b ${PREFIX_ext_sip}/bin -d ${PREFIX_ext_sip}/lib/Python.framework/Versions/Current/site-packages/ 
            -e ${PREFIX_ext_sip}/include --sipdir ${PREFIX_ext_sip}/share/sip --target-py-version 3.8 --sip-module PyQt5.sip
        BUILD_COMMAND make

        # force single-threaded installation to avoid
        # race conditions in post-install .py script
        INSTALL_COMMAND make -j1 install
            # COMMAND ${CMAKE_COMMAND} -E create_symlink ./PyQt5/sip.so ${PREFIX_ext_sip}/lib/Python.framework/Versions/Current/site-packages/sip.so
            # COMMAND ${CMAKE_COMMAND} -E create_symlink ./PyQt5/sip.pyi ${PREFIX_ext_sip}/lib/Python.framework/Versions/Current/site-packages/sip.pyi

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
    )
elseif(UNIX)
    set(Python_EXECUTABLE ${PREFIX_ext_sip}/bin/python3)
    if(NOT EXISTS ${Python_EXECUTABLE})
      message("WARNING: using system python3!")
      set(Python_EXECUTABLE python3)
    endif()

    ExternalProject_Add( ext_sip
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://pypi.python.org/packages/source/s/sip/sip-6.1.1.tar.gz
        URL_HASH SHA256=52d25af2fcd764c4e15cc9cd1350bdb0e63f52dfa2aa3c5e7679af7fde9f7e20

        CONFIGURE_COMMAND ""

        BUILD_COMMAND ${CMAKE_COMMAND} -E env 
            "PYTHONPATH=${KRITA_PYTHONPATH}"
            ${PYTHON_EXECUTABLE_PATH} <SOURCE_DIR>/setup.py build -j ${SUBMAKE_JOBS}

        INSTALL_COMMAND ${CMAKE_COMMAND} -E env 
            "PYTHONPATH=${KRITA_PYTHONPATH}"
            ${PYTHON_EXECUTABLE_PATH} <SOURCE_DIR>/setup.py install --skip-build --prefix=${PREFIX_ext_sip} --optimize=1

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
    )
elseif (WIN32)
    if(MINGW)
        set(_compiler -c mingw32)
    elseif(MSVC)
        set(_compiler -c msvc)
    endif()

    string(REPLACE "/" "\\" _sip_prefix ${PREFIX_ext_sip})
    string(REPLACE "/" "\\" _krita_pythonpath ${KRITA_PYTHONPATH})
    ExternalProject_Add( ext_sip
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://pypi.python.org/packages/source/s/sip/sip-6.1.0.tar.gz
        URL_HASH SHA256=f069d550dd819609e019e5dc58fc5193e081c7f3fb4f7dc8f9be734e34d4e56e

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-MinGW-Distutils-support-hack.patch

        CONFIGURE_COMMAND ""

        BUILD_COMMAND ${CMAKE_COMMAND} -E env 
            "PYTHONPATH=${_krita_pythonpath}"
            ${Python_EXECUTABLE} <SOURCE_DIR>/setup.py build -j ${SUBMAKE_JOBS} ${_compiler}

        # HACK: Create lib\site-packages before installing
        # See e.g. https://access.redhat.com/errata/RHBA-2020:4437
        INSTALL_COMMAND ${CMAKE_COMMAND} -E make_directory ${_sip_prefix}\\lib\\site-packages
            COMMAND ${CMAKE_COMMAND} -E env 
                "PYTHONPATH=${_krita_pythonpath}"
                ${Python_EXECUTABLE} <SOURCE_DIR>/setup.py install --skip-build --prefix ${_sip_prefix} --optimize=1 --install-scripts=${_sip_prefix}\\bin

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
    )
endif()

