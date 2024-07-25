set(EXTPREFIX_perl "${EXTPREFIX}")

ExternalProject_Add(
    ext_strawberryperl
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    # From https://strawberryperl.com/download/5.32.1.1/strawberry-perl-5.32.1.1-32bit-portable.zip
    URL https://files.kde.org/krita/build/dependencies/strawberry-perl-5.32.1.1-32bit-portable.zip
    URL_HASH SHA1=28bca91cadd6651c2b2463db8587c170bf17f2fa

    BUILD_COMMAND ${CMAKE_COMMAND} -E echo Deploying Strawberry Perl portable
    CONFIGURE_COMMAND ""
    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_BINARY_DIR}/Strawberry
            ${EXTPREFIX_perl}/Strawberry
        COMMAND ${CMAKE_COMMAND} -E echo "Removing Strawberry Perl pkg-config"
        COMMAND ${CMAKE_COMMAND} -E rm
            ${EXTPREFIX_perl}/Strawberry/perl/bin/pkg-config.bat
            ${EXTPREFIX_perl}/Strawberry/perl/bin/pkg-config
    UPDATE_COMMAND ""

    SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/Strawberry"
)

set(USE_STRAWBERRYPERL ON CACHE BOOL "Krita will use its own package of strawberryperl" FORCE)    
set(PERL_EXECUTABLE ${EXTPREFIX}/Strawberry/perl/bin/perl.exe CACHE FILEPATH "Path to our own perl executable" FORCE)
string(TOLOWER "$ENV{PATH}" _path)
if (NOT _path MATCHES "strawberry")
    message(FATAL_ERROR "Please add the following directories to PATH: ${EXTPREFIX}\\Strawberry\\perl\\bin")
else()
    message(STATUS "Perl not found, using Strawberry Perl as a fallback.")
endif()
