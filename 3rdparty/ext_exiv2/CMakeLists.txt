SET(PREFIX_ext_exiv2  "${EXTPREFIX}" )

if (ANDROID)
    ExternalProject_Add( ext_exiv2
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://exiv2.org/builds/exiv2-0.27.3-Source.tar.gz
        URL_HASH SHA256=a79f5613812aa21755d578a297874fb59a85101e793edc64ec2c6bd994e3e778

        INSTALL_DIR ${PREFIX_ext_exiv2}
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_exiv2} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DEXIV2_BUILD_SAMPLES=OFF -DICONV_LIBRARY=${PREFIX_ext_exiv2}/lib -DEXIV2_ENABLE_NLS=OFF -DICONV_INCLUDE_DIR=${PREFIX_ext_exiv2}/include -DEXPAT_LIBRARY=$ENV{BUILD_ROOT}/i/lib/libexpat.so -DEXPAT_INCLUDE_DIR=$ENV{BUILD_ROOT}/i/include -DEXIV2_BUILD_EXIV2_COMMAND=OFF
        UPDATE_COMMAND ""
        DEPENDS ext_expat
    )

elseif(WIN32)

    set(WINDOWS_PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/gcccheck.patch
                      COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Fix-building-exiv2-with-TDMGCC-10.3.0.patch)
                
    if (ICONV_DEP)
        # our internal ext_iconv has a bit different API
    
        set(WINDOWS_PATCH_COMMAND 
            ${WINDOWS_PATCH_COMMAND}
            COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/01_fpermissive_error.patch)
    endif()
    

    ExternalProject_Add( ext_exiv2
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://exiv2.org/builds/exiv2-0.27.3-Source.tar.gz
        URL_HASH SHA256=a79f5613812aa21755d578a297874fb59a85101e793edc64ec2c6bd994e3e778

        PATCH_COMMAND ${WINDOWS_PATCH_COMMAND}

        INSTALL_DIR ${PREFIX_ext_exiv2}
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_exiv2} -DCMAKE_LIBRARY_PATH=${PREFIX_ext_exiv2}/lib -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DEXIV2_BUILD_SAMPLES=OFF -DIconv_INCLUDE_DIR=${PREFIX_ext_exiv2}/include -DEXIV2_ENABLE_NLS=OFF -DEXIV2_BUILD_EXIV2_COMMAND=OFF

        UPDATE_COMMAND ""
        DEPENDS ${ICONV_DEP} ${EXPAT_DEP} ${ZLIB_DEP}
    )
else()
    ExternalProject_Add( ext_exiv2

        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://files.kde.org/krita/build/dependencies/exiv2-0.27.3-Source.tar.gz
        URL_HASH SHA256=a7cee019b07f50e8f61f95326964ef57b05dfe1e599dcbb1a945d466219cede0

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Remove-fcf-protection-from-Exiv2.patch

        INSTALL_DIR ${PREFIX_ext_exiv2}
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_exiv2} -DCMAKE_LIBRARY_PATH=${PREFIX_ext_exiv2}/lib -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DEXIV2_BUILD_SAMPLES=OFF -DIconv_INCLUDE_DIR=${PREFIX_ext_exiv2}/include -DEXIV2_ENABLE_NLS=OFF -DEXIV2_BUILD_EXIV2_COMMAND=OFF

        UPDATE_COMMAND ""
        DEPENDS ext_iconv ext_expat
    )
    
endif()
