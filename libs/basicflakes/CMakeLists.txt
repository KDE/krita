add_subdirectory(plugin)

include_directories(
    ${CMAKE_SOURCE_DIR}/libs/widgets
    ${PIGMENT_INCLUDES}
    ${FLAKE_INCLUDES}
)

set(kritabasicflakes_LIB_SRCS
    tools/KoCreatePathTool.cpp
    tools/KoPencilTool.cpp
)

ki18n_wrap_ui( kritabasicflakes_LIB_SRCS
)


add_library(kritabasicflakes SHARED ${kritabasicflakes_LIB_SRCS})
generate_export_header(kritabasicflakes)

target_link_libraries(kritabasicflakes kritawidgets kritaflake kritapigment)

set_target_properties(kritabasicflakes PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)

install(TARGETS kritabasicflakes  ${INSTALL_TARGETS_DEFAULT_ARGS})
