SET(PREFIX_ext_sip "${EXTPREFIX}" )
if (UNIX)
    set(PYTHON_EXECUTABLE_PATH ${PREFIX_ext_sip}/bin/python3)
    if(NOT EXISTS ${PYTHON_EXECUTABLE_PATH})
      message("WARNING: using system python3!")
      set(PYTHON_EXECUTABLE_PATH python3)
    endif()

    ExternalProject_Add( ext_sip
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://pypi.python.org/packages/source/s/sip/sip-6.1.0.tar.gz
        URL_HASH SHA256=f069d550dd819609e019e5dc58fc5193e081c7f3fb4f7dc8f9be734e34d4e56e

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
            ${PYTHON_EXECUTABLE} <SOURCE_DIR>/setup.py build -j ${SUBMAKE_JOBS} ${_compiler}

        # HACK: Create lib\site-packages before installing
        # See e.g. https://access.redhat.com/errata/RHBA-2020:4437
        INSTALL_COMMAND ${CMAKE_COMMAND} -E make_directory ${_sip_prefix}\\lib\\site-packages
            COMMAND ${CMAKE_COMMAND} -E env 
                "PYTHONPATH=${_krita_pythonpath}"
                ${PYTHON_EXECUTABLE} <SOURCE_DIR>/setup.py install --skip-build --prefix ${_sip_prefix} --optimize=1 --install-scripts=${_sip_prefix}\\bin

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
    )
endif()

