SET(EXTPREFIX_lager "${EXTPREFIX}" )

ExternalProject_Add( ext_zug
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    GIT_REPOSITORY https://github.com/arximboldi/zug.git
    GIT_TAG deb266f4c7c35d325de7eb3d033f06e0809495f2

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_lager} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
)

ExternalProject_Add( ext_immer
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    GIT_REPOSITORY https://github.com/arximboldi/immer.git
    GIT_TAG 42e6beafed53f2ecd971360270f421f9c2e36642

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_lager} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
)

set(EXTRA_LAGER_ARGS)

if (MINGW AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    string(REGEX REPLACE "([0-9])\\.([0-9])(\\.[0-9])?" "\\1"
        KRITA_boost_COMPILER_VERSION ${CMAKE_CXX_COMPILER_VERSION})
    set(EXTRA_LAGER_ARGS "-DBoost_COMPILER=mgw${KRITA_boost_COMPILER_VERSION}")
endif()

ExternalProject_Add( ext_lager
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    GIT_REPOSITORY https://github.com/dimula73/lager.git
    GIT_TAG 0b6ab3e0e880bc36be5da4984d768fde03b7cf19

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Correctly-use-global-namespace-to-resolve-lager-deta.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0002-Fix-attempt-to-use-final-specifier-with-a-non-virtua.patch

    CMAKE_ARGS -Dlager_BUILD_DEBUGGER_EXAMPLES=OFF
       -Dlager_BUILD_TESTS=OFF
       -Dlager_BUILD_EXAMPLES=OFF
       -Dlager_BUILD_DOCS=OFF
       ${EXTRA_LAGER_ARGS}
       -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_lager}
       -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
       ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
    DEPENDS ext_zug ext_immer
)
