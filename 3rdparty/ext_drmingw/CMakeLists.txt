SET (PREFIX_ext_drmingw "${EXTPREFIX}")
if (MINGW)
    ExternalProject_Add(ext_drmingw
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://files.kde.org/krita/build/dependencies/drmingw-25e5211.zip
        URL_HASH SHA1=d7c0480344801b431146ee29ff3a18e43bd29271

        INSTALL_DIR ${PREFIX_ext_drmingw}
        CMAKE_ARGS -DCMAKE_DISABLE_FIND_PACKAGE_PythonInterp=1 -DCMAKE_DISABLE_FIND_PACKAGE_WinDbg=1 -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_drmingw} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

        UPDATE_COMMAND ""
    )
    if ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        ExternalProject_Add(ext_drmingw_deps
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://github.com/jrfonseca/drmingw/releases/download/0.8.1/drmingw-0.8.1-win64.7z
            URL_HASH SHA1=8be29cc7efc0d611056c3b745e7f8de30d7c8baa

            INSTALL_DIR ${PREFIX_ext_drmingw}
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ${CMAKE_COMMAND} -E echo Deploying Dr. Mingw 64-bit dependencies
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/dbghelp.dll ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/symsrv.dll ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/symsrv.yes ${PREFIX_ext_drmingw}/bin/

            UPDATE_COMMAND ""
        )
    else ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        ExternalProject_Add(ext_drmingw_deps
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://github.com/jrfonseca/drmingw/releases/download/0.8.1/drmingw-0.8.1-win32.7z
            URL_HASH SHA1=5eefeefc392cae36afb891ca81e3756aa0d79644

            INSTALL_DIR ${PREFIX_ext_drmingw}
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ${CMAKE_COMMAND} -E echo Deploying Dr. Mingw 32-bit dependencies
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/dbghelp.dll ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/symsrv.dll ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/symsrv.yes ${PREFIX_ext_drmingw}/bin/

            UPDATE_COMMAND ""
        )
    endif ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")

    add_dependencies(ext_drmingw ext_drmingw_deps)
endif (MINGW)
