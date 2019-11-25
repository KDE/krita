SET(PREFIX_ext_ilmbase "${EXTPREFIX}" )
ExternalProject_Add( ext_ilmbase
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/openexr/openexr/releases/download/v2.3.0/ilmbase-2.3.0.tar.gz
        URL_MD5 354bf86de3b930ab87ac63619d60c860

        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_ilmbase} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DNAMESPACE_VERSIONING=OFF

        UPDATE_COMMAND ""
)
if (MINGW)
   ExternalProject_Add_Step(
           ext_ilmbase
           post_install
           COMMAND ${CMAKE_COMMAND} -E copy ${PREFIX_ext_ilmbase}/lib/libHalf.dll ${PREFIX_ext_ilmbase}/bin/libHalf.dll
           COMMAND ${CMAKE_COMMAND} -E copy ${PREFIX_ext_ilmbase}/lib/libIex.dll ${PREFIX_ext_ilmbase}/bin/libIex.dll
           COMMAND ${CMAKE_COMMAND} -E copy ${PREFIX_ext_ilmbase}/lib/libImath.dll ${PREFIX_ext_ilmbase}/bin/libImath.dll
           COMMAND ${CMAKE_COMMAND} -E copy ${PREFIX_ext_ilmbase}/lib/libIlmThread.dll ${PREFIX_ext_ilmbase}/bin/libIlmThread.dll
           COMMAND ${CMAKE_COMMAND} -E copy ${PREFIX_ext_ilmbase}/lib/libIexMath.dll ${PREFIX_ext_ilmbase}/bin/libIexMath.dll
           DEPENDEES install
   )
endif()
