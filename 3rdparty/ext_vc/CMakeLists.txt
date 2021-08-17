SET(PREFIX_ext_vc "${EXTPREFIX}" )

if(APPLE)
    if(MACOS_UNIVERSAL OR apple_has_arm64_optional)
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
	if (MSVC)
		ExternalProject_Add( ext_vc
			DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
			URL https://github.com/VcDevel/Vc/archive/953035b43f76405bb0a54574cd0fb0058570a0c1.zip
			URL_HASH SHA256=77ba3e76118e38f32a7511d16f30881f76e2a49a8e29eab3d117ea5a2c3d49ef

			PATCH_COMMAND  ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Add-missing-MSVC-detection-artifact.patch
				COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0002-Add-MSVC-call-convention.patch
				COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0003-Define-architectures-only-for-each-target.patch

			INSTALL_DIR ${PREFIX_ext_vc}

			CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_vc} -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DCMAKE_SYSTEM_PROCESSOR=x86

			UPDATE_COMMAND ""
		)
	else(MSVC)
		ExternalProject_Add( ext_vc
			DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
			URL https://github.com/VcDevel/Vc/releases/download/1.3.3/Vc-1.3.3.tar.gz
			URL_HASH SHA1=3d3ddd29eca7c2b541fd8d0f00923e57f58d5ef0

			PATCH_COMMAND  ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Workaround-AVX-argument-failures.patch
				  COMMAND  ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/relocatable-vc-1.3-install.patch
                                  COMMAND  ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Fix-Vc-s-headers-to-support-GCC11.patch

			INSTALL_DIR ${PREFIX_ext_vc}

			CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_vc} -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DCMAKE_SYSTEM_PROCESSOR=x86

			UPDATE_COMMAND ""
		)
	endif(MSVC)
else()
    ExternalProject_Add( ext_vc
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/VcDevel/Vc/releases/download/1.4.1/Vc-1.4.1.tar.gz
        URL_HASH SHA1=46e852ab69192cf017a31d7c6d0b35d8aa8fea2d

        PATCH_COMMAND  ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Fix-Vc-s-headers-to-support-GCC11.patch

        INSTALL_DIR ${PREFIX_ext_vc}

        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_vc} -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DCMAKE_SYSTEM_PROCESSOR=x86

        UPDATE_COMMAND ""
    )
endif()
