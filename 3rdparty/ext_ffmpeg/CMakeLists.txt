SET(PREFIX_ext_ffmpeg "${EXTPREFIX}" )

if(MINGW)
    if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        ExternalProject_Add( ext_ffmpeg
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://ffmpeg.zeranoe.com/builds/win64/static/ffmpeg-4.2.1-win64-static.zip
            URL_MD5 ae19c3a0006e4f1f34c5f3910bfb3635

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
            URL https://ffmpeg.zeranoe.com/builds/win32/static/ffmpeg-4.2.1-win32-static.zip
            URL_MD5 6a437c84d498ee77df69e446d8384bd4

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
