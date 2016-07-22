SET(PREFIX_ext_expat "${EXTPREFIX}" )

if (MSVC OR MINGW)
ExternalProject_Add( ext_expat
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
	URL http://files.kde.org/krita/build/dependencies/expat-2.2.0.tar.bz2
	URL_MD5 2f47841c829facb346eb6e3fab5212e2

	PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/expat-2.1.0-20130311.diff
	INSTALL_DIR ${PREFIX_ext_expat}
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_expat} -DBUILD_tests=OFF -DBUILD_examples=OFF -DBUILD_tools=OFF -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}
	UPDATE_COMMAND ""

	ALWAYS 0
	DEPENDS ext_patch
)

else()

ExternalProject_Add( ext_expat
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
	URL http://files.kde.org/krita/build/dependencies/expat-2.2.0.tar.bz2
	URL_MD5 2f47841c829facb346eb6e3fab5212e2

	INSTALL_DIR ${PREFIX_ext_expat}
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_expat}  ${GLOBAL_AUTOMAKE_PROFILE}
    BUILD_COMMAND make
    INSTALL_COMMAND make install

	UPDATE_COMMAND ""
	ALWAYS 0

)

endif()
