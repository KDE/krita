SET(PREFIX_ext_png2ico "${EXTPREFIX}" )
ExternalProject_Add( ext_png2ico
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
	URL http://files.kde.org/krita/build/dependencies/png2ico.zip
	URL_MD5 b9c8b31d62f4b2e4f1887bbb63e8a905

	INSTALL_DIR ${PREFIX_ext_png2ico}
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ${CMAKE_COMMAND} -E echo deploying png2ico.exe binary
	INSTALL_COMMAND ${CMAKE_COMMAND} -E make_directory ${PREFIX_ext_png2ico}/bin
		COMMAND ${CMAKE_COMMAND} -E copy bin/png2ico.exe ${PREFIX_ext_patch}/bin/png2ico.exe

	UPDATE_COMMAND ""
	BUILD_IN_SOURCE 1
	ALWAYS 0
)
