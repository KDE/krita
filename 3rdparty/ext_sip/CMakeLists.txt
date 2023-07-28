SET(PREFIX_ext_sip "${EXTPREFIX}" )

if(UNIX)
    ExternalProject_Add( ext_sip
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://pypi.python.org/packages/source/s/sip/sip-6.6.2.tar.gz
        URL_HASH SHA256=0e3efac1c5dfd8e525ae57140927df26993e13f58b89d1577c314f4105bfd90d

        CONFIGURE_COMMAND ""

        BUILD_COMMAND ${CMAKE_COMMAND} -E env 
            "PYTHONPATH=${KRITA_PYTHONPATH}"
            ${Python_EXECUTABLE} <SOURCE_DIR>/setup.py build -j ${SUBMAKE_JOBS}

        INSTALL_COMMAND ${CMAKE_COMMAND} -E env 
            "PYTHONPATH=${KRITA_PYTHONPATH}"
            ${Python_EXECUTABLE} <SOURCE_DIR>/setup.py install --skip-build --prefix=${PREFIX_ext_sip} --optimize=1

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
    )
elseif (WIN32)
    if(MINGW)
        set(_compiler -c mingw32)
    elseif(MSVC)
        set(_compiler -c msvc)
    endif()

    krita_to_native_path("${PREFIX_ext_sip}" _sip_prefix)
    krita_to_native_path("${KRITA_PYTHONPATH}" _krita_pythonpath)
    string(TOLOWER ${_krita_pythonpath} _krita_pythonpath)
    krita_to_native_environment_path_list("${_krita_pythonpath}" _krita_pythonpath)

    ExternalProject_Add( ext_sip
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://pypi.python.org/packages/source/s/sip/sip-6.6.2.tar.gz
        URL_HASH SHA256=0e3efac1c5dfd8e525ae57140927df26993e13f58b89d1577c314f4105bfd90d

        CONFIGURE_COMMAND ""

        BUILD_COMMAND ${CMAKE_COMMAND} -E env 
            PYTHONPATH=${_krita_pythonpath}
            SETUPTOOLS_USE_DISTUTILS=local
            ${Python_EXECUTABLE} <SOURCE_DIR>/setup.py build -j ${SUBMAKE_JOBS} ${_compiler}

        # HACK: Create lib\site-packages before installing
        # See e.g. https://access.redhat.com/errata/RHBA-2020:4437
        INSTALL_COMMAND ${CMAKE_COMMAND} -E make_directory ${_sip_prefix}\\lib\\site-packages
            COMMAND ${CMAKE_COMMAND} -E env 
                PYTHONPATH=${_krita_pythonpath}
                SETUPTOOLS_USE_DISTUTILS=local
                ${Python_EXECUTABLE} <SOURCE_DIR>/setup.py install --skip-build --prefix=${_sip_prefix} --optimize=1 --install-scripts=${_sip_prefix}\\bin

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
    )

    if (MINGW)
        ExternalProject_Add_Step(ext_sip
            wheel_update_setuptools
            COMMAND ${CMAKE_COMMAND} -E env 
            PYTHONPATH=${_krita_pythonpath}
            PYTHONUSERBASE=${_sip_prefix}
            ${Python_EXECUTABLE} -m pip install -U pip setuptools>==59.8.0
                COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/../ext_python/sitecustomize.py ${_sip_prefix}\\lib\\site-packages

            DEPENDERS build
        )
        message(STATUS "Using mingw, Will deploy workaround for setuptools.")
        ExternalProject_Add_Step(ext_sip
            distutils_system_mod
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/../ext_python/_distutils_system_mod.py ${_sip_prefix}\\lib\\site-packages

            DEPENDERS build
        )
    endif()
endif()

