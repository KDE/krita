SET(PREFIX_ext_boost "${EXTPREFIX}" )
if (MSVC)
	if (${CMAKE_GENERATOR} STREQUAL "Visual Studio 14 2015 Win64")
		ExternalProject_Add(
			ext_boost
			DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
			URL http://files.kde.org/krita/build/dependencies/boost_1_61_0.zip
			URL_MD5 015ae4afa6f3e597232bfe1dab949ace

			CONFIGURE_COMMAND <SOURCE_DIR>/bootstrap.bat --prefix=${PREFIX_ext_boost}
			BUILD_COMMAND <SOURCE_DIR>/b2.exe --with-system --build-dir=build-dir --prefix=${PREFIX_ext_boost} toolset=msvc-14.0 variant=release link=shared  threading=multi architecture=x86 address-model=64 variant=release install
			INSTALL_COMMAND ""
			INSTALL_DIR ${EXTPREFIX_boost}

			UPDATE_COMMAND ""
			ALWAYS 0
			BUILD_IN_SOURCE 1
		)
		ExternalProject_Add_Step(
			ext_boost
			post_install
			COMMAND ${CMAKE_COMMAND} -E copy ${PREFIX_ext_boost}/lib/boost_system-vc140-mt-1_55.dll ${PREFIX_ext_boost}/bin/boost_system-vc140-mt-1_55.dll
			DEPENDEES install
		)
		
	else()
		ExternalProject_Add(
			ext_boost

			DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
			URL http://files.kde.org/krita/build/dependencies/boost_1_61_0.zip
			URL_MD5 015ae4afa6f3e597232bfe1dab949ace

			CONFIGURE_COMMAND <SOURCE_DIR>/bootstrap.bat --prefix=${PREFIX_ext_boost}
			BUILD_COMMAND <SOURCE_DIR>/b2.exe --with-system --build-dir=build-dir --prefix=${PREFIX_ext_boost} toolset=msvc-14.0 variant=release link=shared  threading=multi architecture=x86 variant=release install
			INSTALL_COMMAND ""
			INSTALL_DIR ${EXTPREFIX_boost}

			UPDATE_COMMAND ""
			ALWAYS 0
			BUILD_IN_SOURCE 1
        )
        ExternalProject_Add_Step(
			ext_boost
			post_install
			COMMAND ${CMAKE_COMMAND} -E copy ${PREFIX_ext_boost}/lib/boost_system-vc140-mt-1_55.dll ${PREFIX_ext_boost}/bin/boost_system-vc140-mt-1_55.dll
			DEPENDEES install
		)
	endif()
elseif(MINGW)
   ExternalProject_Add(
       ext_boost

       DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
       URL http://files.kde.org/krita/build/dependencies/boost_1_61_0.zip
       URL_MD5 015ae4afa6f3e597232bfe1dab949ace

       CONFIGURE_COMMAND <SOURCE_DIR>/bootstrap.bat mingw --prefix=${PREFIX_ext_boost}
       BUILD_COMMAND <SOURCE_DIR>/b2.exe --with-system --build-dir=build-dir --prefix=${PREFIX_ext_boost} toolset=gcc variant=release link=shared  threading=multi architecture=x86 variant=release install
       INSTALL_COMMAND ""
       INSTALL_DIR ${EXTPREFIX_boost}

       UPDATE_COMMAND ""
       ALWAYS 0
       BUILD_IN_SOURCE 1
   )
   ExternalProject_Add_Step(
       ext_boost
       post_install
       COMMAND ${CMAKE_COMMAND} -E copy ${PREFIX_ext_boost}/lib/libboost_system-mgw51-mt-1_55.dll ${PREFIX_ext_boost}/bin/
       DEPENDEES install
   )
else()
	ExternalProject_Add( ext_boost
        	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
	        URL http://files.kde.org/krita/build/dependencies/boost_1_61_0.tar.bz2
        	URL_MD5 6095876341956f65f9d35939ccea1a9f

        	CONFIGURE_COMMAND <SOURCE_DIR>/bootstrap.sh --prefix=${PREFIX_ext_boost} --with-libraries=system
	        BUILD_COMMAND <SOURCE_DIR>/b2 install
	        INSTALL_COMMAND ""
        	INSTALL_DIR ${PREFIX_ext_boost}

	        UPDATE_COMMAND ""
        	ALWAYS 0
	        BUILD_IN_SOURCE 1
)
endif()
