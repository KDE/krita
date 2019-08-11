SET(PREFIX_ext_ffmpeg "${EXTPREFIX}" )

if(MINGW)
    if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        ExternalProject_Add( ext_ffmpeg
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://ffmpeg.zeranoe.com/builds/win64/static/ffmpeg-4.0-win64-static.zip
            URL_MD5 bf496481c6991c529e2e94a8e0fa3113

            INSTALL_DIR ${PREFIX_ext_ffmpeg}
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ${CMAKE_COMMAND} -E echo deploying ffmpeg3 64-bit binary
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/ffmpeg.exe ${PREFIX_ext_ffmpeg}/bin/ffmpeg.exe
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/LICENSE.txt ${PREFIX_ext_ffmpeg}/bin/ffmpeg_LICENSE.txt
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/README.txt ${PREFIX_ext_ffmpeg}/bin/ffmpeg_README.txt
            UPDATE_COMMAND ""
        )
    else("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        ExternalProject_Add( ext_ffmpeg
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://ffmpeg.zeranoe.com/builds/win32/static/ffmpeg-4.0-win32-static.zip
            URL_MD5 a969a969e3404fe35100e85a37186e5f

            INSTALL_DIR ${PREFIX_ext_ffmpeg}
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ${CMAKE_COMMAND} -E echo deploying ffmpeg3 32-bit binary
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/ffmpeg.exe ${PREFIX_ext_ffmpeg}/bin/ffmpeg.exe
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/LICENSE.txt ${PREFIX_ext_ffmpeg}/bin/ffmpeg_LICENSE.txt
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/README.txt ${PREFIX_ext_ffmpeg}/bin/ffmpeg_README.txt
            UPDATE_COMMAND ""
        )
    endif("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")

endif()
