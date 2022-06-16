add_subdirectory(tests)

set(XCFTOOLS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/xcftools")

include_directories( ${XCFTOOLS_SOURCE_DIR})

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUC)
    list(APPEND COMPILE_FLAGS -Wno-undef -Wno-missing-format-attribute -Wno-sign-compare)
endif ()

if (CMAKE_CXX_COMPILER_ID MATCHES "[cC][lL][aA][nN][gG]")
    add_compile_options("-Wno-undef")
    add_compile_options("-Wno-cast-align")
    add_compile_options("-Wno-sign-compare")
endif ()

if (CMAKE_COMPILER_IS_GNUCC)
    add_compile_options("-Wno-suggest-attribute=format")
    add_compile_options("-Wno-sign-compare")
endif()

if (ANDROID)
    add_compile_options("-std=gnu89")
endif()

set(kritaxcfimport_SOURCES
    kis_xcf_import.cpp
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


add_library(kritaxcfimport MODULE ${kritaxcfimport_SOURCES})

target_link_libraries(kritaxcfimport kritaui )

if (WIN32)
    target_link_libraries(kritaxcfimport kritaui ${WIN32_PLATFORM_NET_LIBS})
endif ()

install(TARGETS kritaxcfimport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install( PROGRAMS  krita_xcf.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
