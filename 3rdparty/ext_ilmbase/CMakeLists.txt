SET(PREFIX_ext_ilmbase "${EXTPREFIX}" )
ExternalProject_Add( ext_ilmbase
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL http://files.kde.org/krita/build/dependencies/ilmbase-2.2.1.tar.gz
        URL_MD5 7b86128b04f0541b6bb33633e299cb44

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
