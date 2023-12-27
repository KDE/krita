# SPDX-FileCopyrightText: 2023 Ivan Santa Maria <ghevan@gmail.com>
# SPDX-License-Ref: BSD-3-Clause

include(ExternalProject)

set(KRITA_MACOS_EXTERNAL_PACKAGE_NAME "package_build_fat.tar.gz")
# used always for stepped universal or non universal builds.
set(PARENT_CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR})
set(KRITA_MACOS_CONSOLIDATE_SCRIPT ${PARENT_CMAKE_SOURCE_DIR}/tools/macos_consolidate.sh)

set(kis_ONE_VALUE_ARGS URL URL_HASH DOWNLOAD_DIR WORKING_DIRECTORY
    GIT_REPOSITORY GIT_TAG GIT_REMOTE_UPDATE_STRATEGY GIT_SUBMODULES_RECURSE GIT_PROGRESS
    INSTALL_DIR
    )
set(kis_MULTI_VALUE_ARGS PATCH_COMMAND CONFIGURE_ARGS CMAKE_ARGS DEPENDS
    GIT_SUBMODULES
    CONFIGURE_COMMAND BUILD_COMMAND INSTALL_COMMAND UPDATE_COMMAND
    )


# If APPLE call the muti build arch macro
#    in other case pass args to ExternalProject_Add
#    or if MESON given, make use of kis_ExternalProject_Add_meson
macro(kis_ExternalProject_Add_with_separate_builds_apple)

    if(APPLE)
        kis_ExternalProject_Add_macos(${ARGN})
    else()
        set(options MESON AUTOMAKE)
        cmake_parse_arguments(EXT "${options}" "" "" ${ARGN})
        
        if(${EXT_MESON})
            kis_ExternalProject_Add_meson(${EXT_UNPARSED_ARGUMENTS})
        else()
            # parsed arguments will not pass to child macro
            set(oneValueArgs WORKING_DIRECTORY)
            set(multiValueArgs CONFIGURE_ARGS)
            cmake_parse_arguments(EXT "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

            ExternalProject_Add(${EXT_UNPARSED_ARGUMENTS})
        endif()
    endif()
endmacro()


# Convenience macro to set all common MESON configuration
macro(kis_ExternalProject_Add_meson EXT_NAME)
    set(onValueArgs "${kis_ONE_VALUE_ARGS}")
    set(multiValueArgs "${kis_MULTI_VALUE_ARGS}")
    cmake_parse_arguments(EXT "${options}" "${onValueArgs}" "${multiValueArgs}" ${ARGN})

    ExternalProject_Add(
        ${EXT_NAME}
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL ${EXT_URL}
        URL_HASH ${EXT_URL_HASH}
        GIT_REPOSITORY ${EXT_GIT_REPOSITORY}
        GIT_TAG ${EXT_GIT_TAG}

        PATCH_COMMAND ${EXT_PATCH_COMMAND}

        CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env
            PYTHONPATH=${_krita_pythonpath}
            PKG_CONFIG_ALLOW_SYSTEM_CFLAGS=1
            ${MESON_BINARY_PATH} setup <BINARY_DIR> <SOURCE_DIR>
                --libdir=lib
                -Dbuildtype=$<IF:$<CONFIG:Debug>,debug,debugoptimized>
                ${EXT_CONFIGURE_ARGS}

        BUILD_COMMAND ${CMAKE_COMMAND} -E env
            PYTHONPATH=${_krita_pythonpath}
            ${MESON_BINARY_PATH} compile -C <BINARY_DIR> -j${SUBMAKE_JOBS}

        INSTALL_COMMAND ${CMAKE_COMMAND} -E env
            PYTHONPATH=${_krita_pythonpath}
            ${MESON_BINARY_PATH} install -C <BINARY_DIR>

        UPDATE_COMMAND ""

        DEPENDS ${EXT_DEPENDS}
    )
endmacro()


# simplifies double arch compile con macos for external libraries not
# supporting one step universal build
# only define CONFIGURE, BUILD, INSTALL for custom build steps
macro(kis_ExternalProject_Add_macos EXT_NAME)
    set(options MESON AUTOMAKE)
    set(onValueArgs URL URL_HASH DOWNLOAD_DIR WORKING_DIRECTORY
        GIT_REPOSITORY GIT_TAG GIT_REMOTE_UPDATE_STRATEGY GIT_SUBMODULES_RECURSE GIT_PROGRESS
        INSTALL_DIR
    )
    set(multiValueArgs PATCH_COMMAND CONFIGURE_ARGS CMAKE_ARGS DEPENDS
        GIT_SUBMODULES
        CONFIGURE_COMMAND BUILD_COMMAND INSTALL_COMMAND UPDATE_COMMAND
    )
    cmake_parse_arguments(EXT "${options}" "${onValueArgs}" "${multiValueArgs}" ${ARGN})


    macro(compile_arch ARCH DEPENDEES EXT_MESON)
        # Some packages need special configure to set architecture
        string(REPLACE "@ARCH@" ${ARCH} EXT_CONFIGURE_COMMAND_ARCH "${EXT_CONFIGURE_COMMAND}")
        string(REPLACE "@ARCH@" ${ARCH} EXT_WORKING_DIRECTORY_ARCH "${EXT_WORKING_DIRECTORY}")

        if(NOT EXT_WORKING_DIRECTORY_ARCH)
            set(EXT_WORKING_DIRECTORY_ARCH <BINARY_DIR>)
        endif()

        # CMake variant
        set(MAC_CONFIGURE_COMMAND
            arch -${ARCH} ${CMAKE_COMMAND} -S <SOURCE_DIR> -B <BINARY_DIR>-${ARCH}
            ${EXT_CONFIGURE_ARGS}
            ${EXT_CMAKE_ARGS}
            -DCMAKE_OSX_ARCHITECTURES:STRING=${ARCH}
        )
        set(MAC_BUILD_COMMAND
            arch -${ARCH} ${CMAKE_COMMAND} --build <BINARY_DIR>-${ARCH} -j${SUBMAKE_JOBS}
        )
        set(MAC_INSTALL_COMMAND
            DESTDIR=<TMP_DIR>/build-${ARCH} ${CMAKE_COMMAND} --install <BINARY_DIR>-${ARCH}
        )
        # Meson variant
        if(${EXT_MESON})
            set(MAC_CONFIGURE_COMMAND
                ${CMAKE_COMMAND} -E env
                PYTHONPATH=${_krita_pythonpath}
                PKG_CONFIG_ALLOW_SYSTEM_CFLAGS=1
                arch -${ARCH} ${MESON_BINARY_PATH} setup <BINARY_DIR>-${ARCH} <SOURCE_DIR>
                    --libdir=lib
                    -Dbuildtype=$<IF:$<CONFIG:Debug>,debug,debugoptimized>
                    ${EXT_CONFIGURE_ARGS}
                    ${EXT_CMAKE_ARGS}
                    --native-file=${CMAKE_CURRENT_BINARY_DIR}/../meson-compiler_${ARCH}.ini
            )
            set(MAC_BUILD_COMMAND
                ${CMAKE_COMMAND} -E env
                PYTHONPATH=${_krita_pythonpath}
                arch -${ARCH} ${MESON_BINARY_PATH} compile -C <BINARY_DIR>-${ARCH} -j${SUBMAKE_JOBS}
            )
            set(MAC_INSTALL_COMMAND
                ${CMAKE_COMMAND} -E env
                PYTHONPATH=${_krita_pythonpath}
                DESTDIR=<TMP_DIR>/build-${ARCH}
                ${MESON_BINARY_PATH} install -C <BINARY_DIR>-${ARCH}
            )
        endif()

        if(${EXT_AUTOMAKE} OR EXT_BUILD_COMMAND)
            set(MAC_CONFIGURE_COMMAND
                arch -${ARCH}
                -e "CFLAGS+=-arch ${ARCH}" -e "CXXFLAGS+=-arch ${ARCH}"
                <SOURCE_DIR>/configure
                ${EXT_CONFIGURE_ARGS}
                ${EXT_CMAKE_ARGS}
            )
            set(MAC_BUILD_COMMAND
                make clean COMMAND arch -${ARCH} make -j${SUBMAKE_JOBS}
            )
            set(MAC_INSTALL_COMMAND
                make install DESTDIR=<TMP_DIR>/build-${ARCH} INSTALL_ROOT=<TMP_DIR>/build-${ARCH}
            )
        endif()

        if(EXT_CONFIGURE_COMMAND AND NOT EXT_CONFIGURE_ARGS)
            set(MAC_CONFIGURE_COMMAND arch -${ARCH} ${EXT_CONFIGURE_COMMAND_ARCH})
        endif()
        if(EXT_BUILD_COMMAND)
            set(MAC_BUILD_COMMAND arch -${ARCH} ${EXT_BUILD_COMMAND})
        endif()
        if(EXT_INSTALL_COMMAND)
            # HACK: be over paranoid about DESTDIR 
            set(MAC_INSTALL_COMMAND arch -${ARCH} -e DESTDIR=<TMP_DIR>/build-${ARCH}
                -e INSTALL_ROOT=<TMP_DIR>/build-${ARCH}
                ${EXT_INSTALL_COMMAND}
                DESTDIR=<TMP_DIR>/build-${ARCH} INSTALL_ROOT=<TMP_DIR>/build-${ARCH}
                )
        endif()

        ExternalProject_Add_Step( ${EXT_NAME} build-${ARCH}
            # newline space is important
            COMMENT "Building ${EXT_NAME} ${ARCH}"

            WORKING_DIRECTORY ${EXT_WORKING_DIRECTORY_ARCH}
            # Configure arch variant
            COMMAND ${MAC_CONFIGURE_COMMAND}
            # build
            COMMAND ${MAC_BUILD_COMMAND} 
            # install in tmp creating fat-bin
            COMMAND ${MAC_INSTALL_COMMAND}

            DEPENDEES ${DEPENDEES}
            DEPENDERS build
        )
    endmacro()

    if(NOT EXT_GIT_REPOSITORY)
        ExternalProject_Add( ${EXT_NAME}
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL ${EXT_URL}
            URL_HASH ${EXT_URL_HASH}

            PATCH_COMMAND ${EXT_PATCH_COMMAND}

            CONFIGURE_COMMAND ""
            BUILD_COMMAND ${CMAKE_COMMAND} -E env ${KRITA_MACOS_CONSOLIDATE_SCRIPT} <TMP_DIR>
            INSTALL_COMMAND tar -xzf <TMP_DIR>/${KRITA_MACOS_EXTERNAL_PACKAGE_NAME} -C "/"
            UPDATE_COMMAND ""

            DEPENDS ${EXT_DEPENDS}
        )
    else()
        ExternalProject_Add( ${EXT_NAME}
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            GIT_REPOSITORY ${EXT_GIT_REPOSITORY}
            GIT_TAG ${EXT_GIT_TAG}
            GIT_SUBMODULES  ${EXT_GIT_SUBMODULES}
            GIT_SUBMODULES_RECURSE ${EXT_GIT_SUBMODULES_RECURSE}
            GIT_PROGRESS ${EXT_GIT_PROGRESS}
            GIT_REMOTE_UPDATE_STRATEGY ${EXT_GIT_REMOTE_UPDATE_STRATEGY}

            PATCH_COMMAND ${EXT_PATCH_COMMAND}

            CONFIGURE_COMMAND ""
            BUILD_COMMAND ${CMAKE_COMMAND} -E env ${KRITA_MACOS_CONSOLIDATE_SCRIPT} <TMP_DIR>
            INSTALL_COMMAND tar -xzf <TMP_DIR>/${KRITA_MACOS_EXTERNAL_PACKAGE_NAME} -C "/"
            UPDATE_COMMAND ""

            DEPENDS ${EXT_DEPENDS}
        )
    endif()

    set(step_depend configure)
    foreach(ARCH ${CMAKE_OSX_ARCHITECTURES})
        compile_arch(${ARCH} ${step_depend} ${EXT_MESON})
        set(step_depend build-${ARCH})
    endforeach()
endmacro()

macro(mkdir_build_arch_dir ARCH)
ExternalProject_Add_Step(ext_qt mkdir-build-${ARCH}
    COMMAND ${CMAKE_COMMAND} -E make_directory <BINARY_DIR>-${ARCH}
    DEPENDERS configure
)
endmacro()