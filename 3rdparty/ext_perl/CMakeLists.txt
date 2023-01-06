if(NOT PERL_FOUND AND WIN32)
    ExternalProject_Add(
        ext_strawberryperl
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        # From https://strawberryperl.com/download/5.32.1.1/strawberry-perl-5.32.1.1-32bit-portable.zip
        URL https://files.kde.org/krita/build/dependencies/strawberry-perl-5.32.1.1-32bit-portable.zip
        URL_HASH SHA1=28bca91cadd6651c2b2463db8587c170bf17f2fa

        BUILD_COMMAND ${CMAKE_COMMAND} -E echo Deploying Strawberry Perl portable
        CONFIGURE_COMMAND ""
        INSTALL_COMMAND ""
        UPDATE_COMMAND ""

        SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/Strawberry"
    )
    
    set(USE_STRAWBERRYPERL ON CACHE BOOL "Krita will use its own package of strawberryperl" FORCE)    
    set(KRITA_PERL_EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/Strawberry/perl/bin/perl.exe CACHE FILEPATH "Path to our own perl executable" FORCE)
elseif (PERL_EXECUTABLE) 
    set(KRITA_PERL_EXECUTABLE ${PERL_EXECUTABLE} CACHE FILEPATH "Path to the found perl executable" FORCE)
elseif(NOT PERL_FOUND)
    message(FATAL_ERROR "Perl is not found and it is needed for building of Qt!")
endif()

if (KRITA_PERL_EXECUTABLE) 
    cmake_path(GET KRITA_PERL_EXECUTABLE PARENT_PATH KRITA_PERL_PATH_INTERNAL)
    set(KRITA_PERL_PATH ${KRITA_PERL_PATH_INTERNAL} CACHE PATH "Path to a directory containing perl executable" FORCE)
endif()