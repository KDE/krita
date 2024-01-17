SET(PREFIX_ext_boost "${EXTPREFIX}" )
if(WIN32)
   if(MINGW)

    string(REGEX REPLACE "([0-9])\\.([0-9])(\\.[0-9])?" "\\1"
           KRITA_boost_COMPILER_VERSION ${CMAKE_CXX_COMPILER_VERSION})

   ExternalProject_Add(
       ext_boost

       DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

       URL https://boostorg.jfrog.io/artifactory/main/release/1.80.0/source/boost_1_80_0.tar.gz
        URL_HASH SHA256=4b2136f98bdd1f5857f1c3dea9ac2018effe65286cf251534b6ae20cc45e1847

       PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Fix-Krita-builds-with-Clang.patch
             COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Fix-building-Boost.Build-with-a-chosen-toolset.patch
             COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Fix-compilation-with-clang-15.patch

       CONFIGURE_COMMAND <SOURCE_DIR>/bootstrap.bat gcc --prefix=${PREFIX_ext_boost}
       BUILD_COMMAND <SOURCE_DIR>/b2.exe -j${SUBMAKE_JOBS} linkflags=${SECURITY_SHARED_LINKER_FLAGS} --with-system --build-dir=build-dir --prefix=${PREFIX_ext_boost} toolset=gcc variant=release link=shared threading=multi architecture=x86 address-model=64 install
       INSTALL_COMMAND ""
       INSTALL_DIR ${PREFIX_ext_boost}

       UPDATE_COMMAND ""
       BUILD_IN_SOURCE 1
   )
   ExternalProject_Add_Step(
       ext_boost
       post_install
       COMMAND ${CMAKE_COMMAND} -E copy ${PREFIX_ext_boost}/lib/libboost_system-mgw${KRITA_boost_COMPILER_VERSION}-mt-x64-1_80.dll ${PREFIX_ext_boost}/bin/
       DEPENDEES install
   )
   if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(STATUS "Applying Clang override to Boost...")
    set(_user_config "using gcc : : ${CMAKE_CXX_COMPILER} ;")
    file(WRITE 
      ${CMAKE_CURRENT_BINARY_DIR}/user-config.jam
      "${_user_config}"
    )
    ExternalProject_Add_Step(
        ext_boost
        configure_clang_mingw
        COMMAND ${CMAKE_COMMAND} -E rm <SOURCE_DIR>/project-config.jam
        DEPENDEES configure
        DEPENDERS build
    )
    ExternalProject_Add_Step(
        ext_boost
        patch_clang_mingw
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/user-config.jam <SOURCE_DIR>/tools/build/src/user-config.jam
        DEPENDEES patch
        DEPENDERS configure
    )
   endif()
   else()
      # Boost.Build cannot append arbitrary flags to MSVC from the command line.
      # It becomes unable to detect the target architecture or defaults to x86 only.
      # On top of that, parameterizing the linkflags flag breaks the MinGW build.
      ExternalProject_Add(
        ext_boost
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

        URL https://boostorg.jfrog.io/artifactory/main/release/1.80.0/source/boost_1_80_0.tar.gz
        URL_HASH SHA256=4b2136f98bdd1f5857f1c3dea9ac2018effe65286cf251534b6ae20cc45e1847

        CONFIGURE_COMMAND <SOURCE_DIR>/bootstrap.bat msvc --prefix=${PREFIX_ext_boost}
        BUILD_COMMAND <SOURCE_DIR>/b2.exe -j${SUBMAKE_JOBS} --with-system --build-dir=build-dir --prefix=${PREFIX_ext_boost} toolset=msvc variant=release link=shared threading=multi architecture=x86 install
        INSTALL_COMMAND ""
        INSTALL_DIR ${EXTPREFIX_boost}

        UPDATE_COMMAND ""
        BUILD_IN_SOURCE 1
      )
      if (NOT DEFINED MSVC_TOOLSET_VERSION)
        string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" KRITA_boost_COMPILER ${CMAKE_CXX_COMPILER})
        string(REGEX REPLACE "^([0-9]+)\\.([0-9]).+$" "\\1\\2"
        KRITA_boost_COMPILER_VERSION ${KRITA_boost_COMPILER})
      else()
        set(KRITA_boost_COMPILER_VERSION ${MSVC_TOOLSET_VERSION})
      endif()
      ExternalProject_Add_Step(
        ext_boost
        post_install
        COMMAND ${CMAKE_COMMAND} -E copy ${PREFIX_ext_boost}/lib/boost_system-vc${KRITA_boost_COMPILER_VERSION}-mt-x32-1_80.dll ${PREFIX_ext_boost}/bin/
            COMMAND ${CMAKE_COMMAND} -E copy ${PREFIX_ext_boost}/lib/boost_system-vc${KRITA_boost_COMPILER_VERSION}-mt-x64-1_80.dll ${PREFIX_ext_boost}/bin/
        DEPENDEES install
      )
   endif()
elseif(ANDROID)
    ExternalProject_Add( ext_boost
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/moritz-wundke/Boost-for-Android/archive/4067aed71278e292d1b78d8a13431f01f80012f1.tar.gz
        URL_HASH SHA256=e43f43b33e79741c393cb84c277ff0a5fdb38e775f082d576c62938ef08fb3b3

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Support-Android-NDK-r22b.patch

        CONFIGURE_COMMAND ""
        BUILD_COMMAND <SOURCE_DIR>/build-android.sh --prefix=${PREFIX_ext_boost} --with-libraries=system --boost=1.80.0 --arch=${ANDROID_ABI} ${CMAKE_ANDROID_NDK}
        INSTALL_COMMAND ""
        INSTALL_DIR ${PREFIX_ext_boost}

        UPDATE_COMMAND ""
        BUILD_IN_SOURCE 1
    )
    ExternalProject_Add_Step(
       ext_boost
       post_install
       COMMAND sh -c "cp ${PREFIX_ext_boost}/${ANDROID_ABI}/lib/libboost_system-*-1_80.a ${PREFIX_ext_boost}/${ANDROID_ABI}/lib/libboost_system.a"
       DEPENDEES install
   )
else()
  if(APPLE)
      set(MACOS_BOOST_ARGS architecture=arm+x86 cxxflags=${MACOS_ARCH_FLAGS})
  endif()
	ExternalProject_Add( ext_boost
        	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
          URL https://boostorg.jfrog.io/artifactory/main/release/1.80.0/source/boost_1_80_0.tar.gz
          URL_HASH SHA256=4b2136f98bdd1f5857f1c3dea9ac2018effe65286cf251534b6ae20cc45e1847

          PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Fix-compilation-with-clang-15.patch

        	CONFIGURE_COMMAND <SOURCE_DIR>/bootstrap.sh --prefix=${PREFIX_ext_boost} --with-libraries=system
	        BUILD_COMMAND <SOURCE_DIR>/b2 ${MACOS_BOOST_ARGS} -j${SUBMAKE_JOBS} install
	        INSTALL_COMMAND ""
        	INSTALL_DIR ${PREFIX_ext_boost}

	        UPDATE_COMMAND ""
	        BUILD_IN_SOURCE 1
)
endif()
