SET(PREFIX_ext_boost "${EXTPREFIX}" )
if(WIN32)
   if(MINGW)

    string(REGEX REPLACE "([0-9])\\.([0-9])(\\.[0-9])?" "\\1"
           KRITA_boost_COMPILER_VERSION ${CMAKE_CXX_COMPILER_VERSION})

   ExternalProject_Add(
       ext_boost

       DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

       URL https://boostorg.jfrog.io/artifactory/main/release/1.77.0/source/boost_1_77_0.tar.bz2
        URL_HASH SHA256=fc9f85fc030e233142908241af7a846e60630aa7388de9a5fafb1f3a26840854

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
       COMMAND ${CMAKE_COMMAND} -E copy ${PREFIX_ext_boost}/lib/libboost_system-mgw${KRITA_boost_COMPILER_VERSION}-mt-x64-1_77.dll ${PREFIX_ext_boost}/bin/
       DEPENDEES install
   )
   else()
      # Boost.Build cannot append arbitrary flags to MSVC from the command line.
      # It becomes unable to detect the target architecture or defaults to x86 only.
      # On top of that, parameterizing the linkflags flag breaks the MinGW build.
      ExternalProject_Add(
        ext_boost
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

        URL https://boostorg.jfrog.io/artifactory/main/release/1.71.0/source/boost_1_71_0.tar.bz2
        URL_HASH SHA256=d73a8da01e8bf8c7eda40b4c84915071a8c8a0df4a6734537ddde4a8580524ee

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
        COMMAND ${CMAKE_COMMAND} -E copy ${PREFIX_ext_boost}/lib/boost_system-vc${KRITA_boost_COMPILER_VERSION}-mt-x32-1_71.dll ${PREFIX_ext_boost}/bin/
            COMMAND ${CMAKE_COMMAND} -E copy ${PREFIX_ext_boost}/lib/boost_system-vc${KRITA_boost_COMPILER_VERSION}-mt-x64-1_71.dll ${PREFIX_ext_boost}/bin/
        DEPENDEES install
      )
   endif()
elseif(ANDROID)
    ExternalProject_Add( ext_boost
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/moritz-wundke/Boost-for-Android/archive/dc37be02fd7e4bc87a184d0f6934da5a5beb7f51.tar.gz
        URL_HASH SHA256=a962e2572bd15b5b8fcf5a9331195ebd41954209b556766ae178bb82de5dd0d1

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Update-Boost-URLs-Bintray-shut-down.patch
              COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Support-Android-NDK-r22b.patch

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
       COMMAND sh -c "cp ${PREFIX_ext_boost}/${ANDROID_ABI}/lib/libboost_system-*-1_70.a ${PREFIX_ext_boost}/${ANDROID_ABI}/lib/libboost_system.a"
       DEPENDEES install
   )
else()
	ExternalProject_Add( ext_boost
        	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
	        URL https://boostorg.jfrog.io/artifactory/main/release/1.71.0/source/boost_1_71_0.tar.bz2
        	URL_HASH SHA256=d73a8da01e8bf8c7eda40b4c84915071a8c8a0df4a6734537ddde4a8580524ee

        	CONFIGURE_COMMAND <SOURCE_DIR>/bootstrap.sh --prefix=${PREFIX_ext_boost} --with-libraries=system
	        BUILD_COMMAND <SOURCE_DIR>/b2 -j${SUBMAKE_JOBS} install
	        INSTALL_COMMAND ""
        	INSTALL_DIR ${PREFIX_ext_boost}

	        UPDATE_COMMAND ""
	        BUILD_IN_SOURCE 1
)
endif()
