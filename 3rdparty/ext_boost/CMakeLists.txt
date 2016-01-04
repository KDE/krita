SET(PREFIX_ext_boost "${EXTPREFIX}" )
if (MSVC)
	if (${CMAKE_GENERATOR} STREQUAL "Visual Studio 14 2015 Win64")
		ExternalProject_Add(
			ext_boost
			DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
			URL http://files.kde.org/krita/build/dependencies/boost_1_55_0.tar.bz2
			URL_MD5 d6eef4b4cacb2183f2bf265a5a03a354

			CONFIGURE_COMMAND <SOURCE_DIR>/bootstrap.bat --prefix=${PREFIX_ext_boost}
			BUILD_COMMAND <SOURCE_DIR>/b2.exe --with-system --build-dir=build-dir --prefix=${PREFIX_ext_boost} toolset=msvc variant=release link=shared  threading=multi architecture=x86 address-model=64 variant=release install
			INSTALL_COMMAND ""
			INSTALL_DIR ${EXTPREFIX_boost}

			UPDATE_COMMAND ""
			ALWAYS 0
			BUILD_IN_SOURCE 1
		)
	else()
		ExternalProject_Add(
			ext_boost

			DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
			URL http://files.kde.org/krita/build/dependencies/boost_1_55_0.tar.bz2
			URL_MD5 d6eef4b4cacb2183f2bf265a5a03a354

			CONFIGURE_COMMAND <SOURCE_DIR>/bootstrap.bat --prefix=${PREFIX_ext_boost}
			BUILD_COMMAND <SOURCE_DIR>/b2.exe --with-system --build-dir=build-dir --prefix=${PREFIX_ext_boost} toolset=msvc variant=release link=shared  threading=multi architecture=x86 variant=release install
			INSTALL_COMMAND ""
			INSTALL_DIR ${EXTPREFIX_boost}

			UPDATE_COMMAND ""
			ALWAYS 0
			BUILD_IN_SOURCE 1
        )
	endif()
else()
	ExternalProject_Add( ext_boost
        	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
	        URL http://files.kde.org/krita/build/dependencies/boost_1_55_0.tar.bz2
        	URL_MD5 d6eef4b4cacb2183f2bf265a5a03a354

        	CONFIGURE_COMMAND <SOURCE_DIR>/bootstrap.sh --prefix=${PREFIX_ext_boost} --with-libraries=system
	        BUILD_COMMAND <SOURCE_DIR>/b2 install
	        INSTALL_COMMAND ""
        	INSTALL_DIR ${PREFIX_ext_boost}

	        UPDATE_COMMAND ""
        	ALWAYS 0
	        BUILD_IN_SOURCE 1
)
endif()
