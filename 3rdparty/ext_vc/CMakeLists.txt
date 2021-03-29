SET(PREFIX_ext_vc "${EXTPREFIX}" )

if(APPLE)
    exec_program("sysctl -n hw.optional.x86_64" RETURN_VALUE apple_has_x86_64_optional)
    string(COMPARE EQUAL ${apple_has_x86_64_optional} "" x86_on_arm)
    if(${x86_on_arm})
        set(EXTRA_X86_ON_ARM_PATCHES ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/macos_VcOnARM.patch)
    endif()

    ExternalProject_Add( ext_vc
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/VcDevel/Vc/releases/download/1.4.1/Vc-1.4.1.tar.gz
        URL_HASH SHA1=46e852ab69192cf017a31d7c6d0b35d8aa8fea2d

        PATCH_COMMAND ${EXTRA_X86_ON_ARM_PATCHES}

        INSTALL_DIR ${PREFIX_ext_vc}

        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_vc} -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DCMAKE_SYSTEM_PROCESSOR=x86

        UPDATE_COMMAND ""
    )

elseif (WIN32)

    ExternalProject_Add( ext_vc
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/VcDevel/Vc/releases/download/1.3.3/Vc-1.3.3.tar.gz
        URL_HASH SHA1=3d3ddd29eca7c2b541fd8d0f00923e57f58d5ef0

        PATCH_COMMAND  ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Workaround-AVX-argument-failures.patch
              COMMAND  ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/relocatable-vc-1.3-install.patch

        INSTALL_DIR ${PREFIX_ext_vc}

        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_vc} -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DCMAKE_SYSTEM_PROCESSOR=x86

        UPDATE_COMMAND ""
    )

else()
    ExternalProject_Add( ext_vc
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/VcDevel/Vc/releases/download/1.4.1/Vc-1.4.1.tar.gz
        URL_HASH SHA1=46e852ab69192cf017a31d7c6d0b35d8aa8fea2d

        INSTALL_DIR ${PREFIX_ext_vc}

        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_vc} -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DCMAKE_SYSTEM_PROCESSOR=x86

        UPDATE_COMMAND ""
    )
endif()
