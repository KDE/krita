SET(PREFIX_ext_gmic "${EXTPREFIX}" )

ExternalProject_Add( ext_gmic
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/vanyossi/gmic/releases/download/v3.6.6.2/gmic-3.6.6.2.tar.gz
    URL_HASH SHA256=3ccec8c0cf848b3fdf5a843fe66b793e28c34a2a4f8f9c9a2ce96a5d0840b650

    SOURCE_SUBDIR gmic-qt

    INSTALL_DIR ${PREFIX_ext_gmic}

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_gmic} -DGMIC_QT_HOST=krita-plugin -DENABLE_SYSTEM_GMIC=FALSE -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}
    LIST_SEPARATOR "|"

    UPDATE_COMMAND ""
)

if (APPLE)
    ExternalProject_Add( ext_openmp
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/llvm/llvm-project/releases/download/llvmorg-21.1.0/openmp-21.1.0.src.tar.xz
        URL_HASH SHA256=60d6c4d2019d546aefc029c9b2c4006dc78419dba4cfd0a376ae025be8ff9778

        INSTALL_DIR ${PREFIX_ext_gmic}

        # aligned_alloc not available in macOS<=10.15, remove after support for those versions dropped
        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-force-turn-off-aligned_alloc.patch

        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_gmic} -DOPENMP_STANDALONE_BUILD=ON
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

        UPDATE_COMMAND ""
    )

    ExternalProject_Add_Step(ext_openmp download-cmake-modules
        COMMENT "Downloading llvm-cmake modules"
        WORKING_DIRECTORY <SOURCE_DIR>/cmake

        # we use external cmake exec to check hash and do some retries before hard failing
        COMMAND ${CMAKE_COMMAND}
            -DEXT_TMP_DIR=<TMP_DIR>
            -D URL=https://github.com/llvm/llvm-project/releases/download/llvmorg-21.1.0/cmake-21.1.0.src.tar.xz
            -D URL_SHA=528347c84c3571d9d387b825ef8b07c7ad93e9437243c32173838439c3b6028f
            -P "${CMAKE_CURRENT_SOURCE_DIR}/download_llvm-cmake_modules.cmake"

        DEPENDERS configure
        DEPENDEES patch 
    )

    ExternalProject_Add_StepDependencies(ext_gmic install ext_openmp)
endif()
