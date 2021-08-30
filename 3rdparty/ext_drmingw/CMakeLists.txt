SET (PREFIX_ext_drmingw "${EXTPREFIX}")
if (MINGW)
    ExternalProject_Add(ext_drmingw
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        # mirrored from https://github.com/amyspark/drmingw/releases/download/0.9.4.1/drmingw-0.9.4.1-patched.tar.gz
        URL https://files.kde.org/krita/build/dependencies/drmingw-0.9.4.1-patched.tar.gz
        URL_HASH SHA256=eeab9ec7b3b1e24e13b6dd18045c74be68e3eb1591e8e2d1d34e64b93897f5d6

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Allow-build-with-llvm-dlltool.patch

        INSTALL_DIR ${PREFIX_ext_drmingw}
        CMAKE_ARGS -DCMAKE_DISABLE_FIND_PACKAGE_PythonInterp=1 -DCMAKE_DISABLE_FIND_PACKAGE_WinDbg=1 -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_drmingw} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

        UPDATE_COMMAND ""
    )
    if ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        ExternalProject_Add(ext_drmingw_deps
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://github.com/jrfonseca/drmingw/releases/download/0.9.3/drmingw-0.9.3-win64.7z
            URL_HASH SHA256=abf891115a63c089e1767ca8ea93bbec676485ff37918a6a8fd57b7b05561087

            INSTALL_DIR ${PREFIX_ext_drmingw}
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ${CMAKE_COMMAND} -E echo Deploying Dr. MinGW 64-bit dependencies
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/dbghelp.dll ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/symsrv.dll ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/symsrv.yes ${PREFIX_ext_drmingw}/bin/

            UPDATE_COMMAND ""
        )
    else ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        ExternalProject_Add(ext_drmingw_deps
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://github.com/jrfonseca/drmingw/releases/download/0.9.3/drmingw-0.9.3-win32.7z
            URL_HASH SHA256=66626b88737e926912bc23ff44227399945ffbdc0aaa76b7de57b87e2a00ed7f

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
elseif(MSVC)
    if ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        ExternalProject_Add(ext_drmingw
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://github.com/jrfonseca/drmingw/releases/download/0.9.3/drmingw-0.9.3-win64.7z
            URL_HASH SHA256=abf891115a63c089e1767ca8ea93bbec676485ff37918a6a8fd57b7b05561087

            INSTALL_DIR ${PREFIX_ext_drmingw}
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ${CMAKE_COMMAND} -E echo Deploying Dr. MinGW 64-bit dependencies (MSVC)
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/dbghelp.dll ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/symsrv.dll ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/symsrv.yes ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/exchndl.dll ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/mgwhelp.dll ${PREFIX_ext_drmingw}/bin/

            UPDATE_COMMAND ""
        )
    else ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        ExternalProject_Add(ext_drmingw
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://github.com/jrfonseca/drmingw/releases/download/0.9.3/drmingw-0.9.3-win32.7z
            URL_HASH SHA256=66626b88737e926912bc23ff44227399945ffbdc0aaa76b7de57b87e2a00ed7f

            INSTALL_DIR ${PREFIX_ext_drmingw}
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ${CMAKE_COMMAND} -E echo Deploying Dr. Mingw 32-bit dependencies (MSVC)
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/dbghelp.dll ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/symsrv.dll ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/symsrv.yes ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/exchndl.dll ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/mgwhelp.dll ${PREFIX_ext_drmingw}/bin/

            UPDATE_COMMAND ""
        )
    endif ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
endif (MINGW)
