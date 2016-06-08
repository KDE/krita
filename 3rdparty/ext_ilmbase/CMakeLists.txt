SET(PREFIX_ext_ilmbase "${EXTPREFIX}" )
ExternalProject_Add( ext_ilmbase
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL http://files.kde.org/krita/build/dependencies/ilmbase-2.2.0.tar.gz
        URL_MD5 b540db502c5fa42078249f43d18a4652

        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_ilmbase} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DNAMESPACE_VERSIONING=OFF

        UPDATE_COMMAND ""
        ALWAYS 0
)

if (MSVC)
	ExternalProject_Add_Step(
			ext_ilmbase
			post_install
			COMMAND ${CMAKE_COMMAND} -E copy ${PREFIX_ext_ilmbase}/lib/Half.dll ${PREFIX_ext_ilmbase}/bin/Half.dll
			COMMAND ${CMAKE_COMMAND} -E copy ${PREFIX_ext_ilmbase}/lib/Iex.dll ${PREFIX_ext_ilmbase}/bin/Iex.dll
			COMMAND ${CMAKE_COMMAND} -E copy ${PREFIX_ext_ilmbase}/lib/Imath.dll ${PREFIX_ext_ilmbase}/bin/Imath.dll
			COMMAND ${CMAKE_COMMAND} -E copy ${PREFIX_ext_ilmbase}/lib/IlmThread.dll ${PREFIX_ext_ilmbase}/bin/IlmThread.dll
			COMMAND ${CMAKE_COMMAND} -E copy ${PREFIX_ext_ilmbase}/lib/IexMath.dll ${PREFIX_ext_ilmbase}/bin/IexMath.dll
			DEPENDEES install
	)
endif()
