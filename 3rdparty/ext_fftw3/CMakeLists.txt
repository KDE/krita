SET(PREFIX_ext_fftw3 "${EXTPREFIX}" )
if (MSVC)
    if (${CMAKE_GENERATOR} STREQUAL "Visual Studio 14 2015 Win64")
        ExternalProject_Add( ext_fftw3
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL http://files.kde.org/krita/build/dependencies/fftw-3.3.4-dll64.zip
            URL_MD5 c4ec1e6d84e7a8b1cd61121d11d44ee4

            INSTALL_DIR ${PREFIX_ext_fftw3}
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ${CMAKE_COMMAND} -E echo deploying fftw3 64 binary
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory bin ${PREFIX_ext_fftw3}
                    COMMAND ${CMAKE_COMMAND} -E copy_directory lib ${PREFIX_ext_fftw3}
                    COMMAND ${CMAKE_COMMAND} -E copy_directory include ${PREFIX_ext_fftw3}

            UPDATE_COMMAND ""
            ALWAYS 0
        )
    else()
         ExternalProject_Add( ext_fftw3
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL http://files.kde.org/krita/build/dependencies/fftw-3.3.4-dll32.zip
            URL_MD5 03754fe2a7fbaf983b0cc9a728e0a5b3

            INSTALL_DIR ${PREFIX_ext_fftw3}
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ${CMAKE_COMMAND} -E echo deploying fftw3 32 binary
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory bin ${PREFIX_ext_fftw3}
                    COMMAND ${CMAKE_COMMAND} -E copy_directory lib ${PREFIX_ext_fftw3}
                    COMMAND ${CMAKE_COMMAND} -E copy_directory include ${PREFIX_ext_fftw3}

            UPDATE_COMMAND ""
            ALWAYS 0
        )
    endif()
else (MSVC)
ExternalProject_Add( ext_fftw3
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
	URL http://files.kde.org/krita/build/dependencies/fftw-3.3.4.tar.gz
        URL_MD5 2edab8c06b24feeb3b82bbb3ebf3e7b3
        INSTALL_DIR ${PREFIX_ext_fftw3}
        
        CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_fftw3} ${GLOBAL_AUTOMAKE_PROFILE} --enable-static=false --enable-shared=yes
        BUILD_COMMAND make
        INSTALL_COMMAND make install

        UPDATE_COMMAND ""
        ALWAYS 0
)
endif (MSVC)
