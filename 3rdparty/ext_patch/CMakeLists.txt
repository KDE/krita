SET(PREFIX_ext_patch "${EXTPREFIX}" )
ExternalProject_Add( ext_patch
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
	URL http://files.kde.org/krita/build/dependencies/patch-2.5.9-7-bin.zip
	URL_MD5 b9c8b31d62f4b2e4f1887bbb63e8a905
	INSTALL_DIR ${PREFIX_ext_patch}
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ${CMAKE_COMMAND} -E echo deploying patch.exe binary
	INSTALL_COMMAND ${CMAKE_COMMAND} -E make_directory ${PREFIX_ext_patch}/bin
		COMMAND ${CMAKE_COMMAND} -E copy bin/patch.exe ${PREFIX_ext_patch}/bin/patch.exe
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/patch.exe.manifest ${PREFIX_ext_patch}/bin/patch.exe.manifest
	UPDATE_COMMAND ""
	BUILD_IN_SOURCE 1
)
