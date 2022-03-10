SET(PREFIX_ext_boost "${EXTPREFIX}" )
if(WIN32)
   if(MINGW)

    string(REGEX REPLACE "([0-9])\\.([0-9])(\\.[0-9])?" "\\1"
           KRITA_boost_COMPILER_VERSION ${CMAKE_CXX_COMPILER_VERSION})

   ExternalProject_Add(
       ext_boost

       DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

       URL https://boostorg.jfrog.io/artifactory/main/release/1.78.0/source/boost_1_78_0.tar.bz2
        URL_HASH SHA256=8681f175d4bdb26c52222665793eef08490d7758529330f98d3b29dd0735bccc

       PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Fix-Krita-builds-with-Clang.patch
             COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Fix-building-Boost.Build-with-a-chosen-toolset.patch

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
       COMMAND ${CMAKE_COMMAND} -E copy ${PREFIX_ext_boost}/lib/libboost_system-mgw${KRITA_boost_COMPILER_VERSION}-mt-x64-1_78.dll ${PREFIX_ext_boost}/bin/
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

        URL https://boostorg.jfrog.io/artifactory/main/release/1.78.0/source/boost_1_78_0.tar.bz2
        URL_HASH SHA256=8681f175d4bdb26c52222665793eef08490d7758529330f98d3b29dd0735bccc

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
        COMMAND ${CMAKE_COMMAND} -E copy ${PREFIX_ext_boost}/lib/boost_system-vc${KRITA_boost_COMPILER_VERSION}-mt-x32-1_78.dll ${PREFIX_ext_boost}/bin/
            COMMAND ${CMAKE_COMMAND} -E copy ${PREFIX_ext_boost}/lib/boost_system-vc${KRITA_boost_COMPILER_VERSION}-mt-x64-1_78.dll ${PREFIX_ext_boost}/bin/
        DEPENDEES install
      )
   endif()
elseif(ANDROID)
    ExternalProject_Add( ext_boost
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/moritz-wundke/Boost-for-Android/archive/ccfc74f1c9ceb35d304294d4f1ed3abff43d2e01.zip
        URL_HASH SHA256=717f02b034acfff9a0bfd2eb3533d181a1a7c5d25db1ac1a37df3ce953c9f61d

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Support-Android-NDK-r22b.patch

        CONFIGURE_COMMAND ""
        BUILD_COMMAND <SOURCE_DIR>/build-android.sh --prefix=${PREFIX_ext_boost} --with-libraries=system --boost=1.71.0 --arch=${ANDROID_ABI} ${CMAKE_ANDROID_NDK}
        INSTALL_COMMAND ""
        INSTALL_DIR ${PREFIX_ext_boost}

        UPDATE_COMMAND ""
        BUILD_IN_SOURCE 1
    )
    ExternalProject_Add_Step(
       ext_boost
       post_install
       COMMAND sh -c "cp ${PREFIX_ext_boost}/${ANDROID_ABI}/lib/libboost_system-*-1_71.a ${PREFIX_ext_boost}/${ANDROID_ABI}/lib/libboost_system.a"
       DEPENDEES install
   )
else()
	ExternalProject_Add( ext_boost
        	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
          URL https://boostorg.jfrog.io/artifactory/main/release/1.78.0/source/boost_1_78_0.tar.bz2
          URL_HASH SHA256=8681f175d4bdb26c52222665793eef08490d7758529330f98d3b29dd0735bccc

        	CONFIGURE_COMMAND <SOURCE_DIR>/bootstrap.sh --prefix=${PREFIX_ext_boost} --with-libraries=system
	        BUILD_COMMAND <SOURCE_DIR>/b2 -j${SUBMAKE_JOBS} install
	        INSTALL_COMMAND ""
        	INSTALL_DIR ${PREFIX_ext_boost}

	        UPDATE_COMMAND ""
	        BUILD_IN_SOURCE 1
)
endif()
