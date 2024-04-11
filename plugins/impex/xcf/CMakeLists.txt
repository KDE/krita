add_subdirectory(tests)

set(XCFTOOLS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/xcftools")

include(CheckCSourceCompiles)
include(CheckIncludeFile)
include(CheckSymbolExists)

# Tests for configure.h
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(i?86.*|x86.*|x86_64.*|amd64.*|AMD64.*)")
set(CAN_DO_UNALIGNED_WORDS 1)
else()
set(CAN_DO_UNALIGNED_WORDS 0)
endif()
set(ENABLE_NLS 1)
CHECK_INCLUDE_FILE("arpa/inet.h" HAVE_ARPA_INET_H)
check_symbol_exists(gettext "libintl.h" HAVE_GETTEXT)
check_symbol_exists(iconv "iconv.h" HAVE_ICONV)
CHECK_INCLUDE_FILE("inttypes.h" HAVE_INTTYPES_H)
CHECK_INCLUDE_FILE("netinet/in.h" HAVE_NETINET_IN_H)
check_symbol_exists(strcasecmp "strings.h" HAVE_STRCASECMP)
if (NOT WIN32 AND NOT APPLE)
    set(ICONV_CONST const)
else()
    set(ICONV_CONST "")
endif()
set(PACKAGE_STRING "${CMAKE_PROJECT_NAME}")
set(PACKAGE_VERSION "${KRITA_VERSION_STRING}")
set(PRECOMPUTED_SCALETABLE 1)
check_c_source_compiles("#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <float.h>

int
main ()
{

  ;
  return 0;
}" STDC_HEADERS)
set(WORDS_BIGENDIAN ${CMAKE_WORDS_BIGENDIAN})
# skip inline check for sanity

configure_file(${XCFTOOLS_SOURCE_DIR}/config.h.in config.h)

set(xcftools_SOURCES
    ${XCFTOOLS_SOURCE_DIR}/xcf-general.c
    ${XCFTOOLS_SOURCE_DIR}/utils.c
    ${XCFTOOLS_SOURCE_DIR}/enums.c
    ${XCFTOOLS_SOURCE_DIR}/pixels.c
    ${XCFTOOLS_SOURCE_DIR}/scaletab.c
    ${XCFTOOLS_SOURCE_DIR}/table.c
    ${XCFTOOLS_SOURCE_DIR}/enums.c
    ${XCFTOOLS_SOURCE_DIR}/flatspec.c
    ${XCFTOOLS_SOURCE_DIR}/flatten.c
)

add_library(xcftools STATIC ${xcftools_SOURCES})
set_property(TARGET xcftools PROPERTY C_STANDARD 11)
set_property(TARGET xcftools PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(xcftools INTERFACE ${XCFTOOLS_SOURCE_DIR})

if (WIN32)
    target_link_libraries(xcftools PUBLIC ws2_32)
endif()

if (CMAKE_C_COMPILER_ID MATCHES "GNU")
    target_compile_options(xcftools PRIVATE -Wno-undef -Wno-missing-format-attribute -Wno-sign-compare -Wno-format -Wno-suggest-attribute=format -Wno-format-security)
elseif (CMAKE_C_COMPILER_ID MATCHES "Clang")
    target_compile_options(xcftools PRIVATE -Wno-undef -Wno-cast-align -Wno-sign-compare -Wno-format)
endif ()

set(kritaxcfimport_SOURCES
    kis_xcf_import.cpp
)

kis_add_library(kritaxcfimport MODULE ${kritaxcfimport_SOURCES})

target_link_libraries(kritaxcfimport kritaui xcftools)

install(TARGETS kritaxcfimport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install( PROGRAMS  krita_xcf.desktop  DESTINATION ${KDE_INSTALL_APPDIR})
