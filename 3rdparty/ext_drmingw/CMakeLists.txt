SET (PREFIX_ext_drmingw "${EXTPREFIX}")
if (MINGW)
    ExternalProject_Add(ext_drmingw
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/amyspark/drmingw/releases/download/0.9.7.1/drmingw-0.9.7.1-patched.tar.gz
        URL_HASH SHA256=998848da2f490bfea455f804ccc3f08bc5d5d0fee7813b406c658451ce6da532

        INSTALL_DIR ${PREFIX_ext_drmingw}
        CMAKE_ARGS -DCMAKE_DISABLE_FIND_PACKAGE_PythonInterp=1 -DCMAKE_DISABLE_FIND_PACKAGE_WinDbg=1 -DPOSIX_THREADS=1 -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_drmingw} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

        UPDATE_COMMAND ""
    )
    if ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        ExternalProject_Add(ext_drmingw_deps
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://github.com/jrfonseca/drmingw/releases/download/0.9.7/drmingw-0.9.7-win64.7z
            URL_HASH SHA256=ff1a62ebc9e0662c7ba7f30141efa047e6ee43a5f1a613f671b0971ba772d597

            INSTALL_DIR ${PREFIX_ext_drmingw}
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ${CMAKE_COMMAND} -E echo Deploying Dr. MinGW 64-bit dependencies
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/dbghelp.dll ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/dbgcore.dll ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/symsrv.dll ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/symsrv.yes ${PREFIX_ext_drmingw}/bin/

            UPDATE_COMMAND ""
        )
    else ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        ExternalProject_Add(ext_drmingw_deps
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://github.com/jrfonseca/drmingw/releases/download/0.9.7/drmingw-0.9.7-win32.7z
            URL_HASH SHA256=3668efb8922ce0e3fb94855870f4d5602eda182faa4c013568e86d369eaf862e

            INSTALL_DIR ${PREFIX_ext_drmingw}
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ${CMAKE_COMMAND} -E echo Deploying Dr. Mingw 32-bit dependencies
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/dbghelp.dll ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/dbgcore.dll ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/symsrv.dll ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/symsrv.yes ${PREFIX_ext_drmingw}/bin/

            UPDATE_COMMAND ""
        )
    endif ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")

    add_dependencies(ext_drmingw ext_drmingw_deps)
elseif(MSVC)
    if ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        ExternalProject_Add(ext_drmingw
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://github.com/jrfonseca/drmingw/releases/download/0.9.7/drmingw-0.9.7-win64.7z
            URL_HASH SHA256=ff1a62ebc9e0662c7ba7f30141efa047e6ee43a5f1a613f671b0971ba772d597

            INSTALL_DIR ${PREFIX_ext_drmingw}
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ${CMAKE_COMMAND} -E echo Deploying Dr. MinGW 64-bit dependencies (MSVC)
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/dbghelp.dll ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/dbgcore.dll ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/symsrv.dll ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/symsrv.yes ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/exchndl.dll ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/mgwhelp.dll ${PREFIX_ext_drmingw}/bin/

            UPDATE_COMMAND ""
        )
    else ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        ExternalProject_Add(ext_drmingw
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://github.com/jrfonseca/drmingw/releases/download/0.9.7/drmingw-0.9.7-win32.7z
            URL_HASH SHA256=3668efb8922ce0e3fb94855870f4d5602eda182faa4c013568e86d369eaf862e

            INSTALL_DIR ${PREFIX_ext_drmingw}
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ${CMAKE_COMMAND} -E echo Deploying Dr. Mingw 32-bit dependencies (MSVC)
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/dbghelp.dll ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/dbgcore.dll ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/symsrv.dll ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/symsrv.yes ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/exchndl.dll ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/mgwhelp.dll ${PREFIX_ext_drmingw}/bin/

            UPDATE_COMMAND ""
        )
    endif ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
endif (MINGW)
