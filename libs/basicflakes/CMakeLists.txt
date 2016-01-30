add_subdirectory(plugin)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/tools
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin
)

set(kritabasicflakes_LIB_SRCS
    tools/KoCreatePathTool.cpp
    tools/KoPencilTool.cpp
)

ki18n_wrap_ui( kritabasicflakes_LIB_SRCS
)


add_library(kritabasicflakes SHARED ${kritabasicflakes_LIB_SRCS})
generate_export_header(kritabasicflakes)

target_include_directories(kritabasicflakes
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tools>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/plugin>
)

target_link_libraries(kritabasicflakes 
    PUBLIC
        kritawidgets 
        kritaflake 
        kritapigment
)

set_target_properties(kritabasicflakes PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)

install(TARGETS kritabasicflakes  ${INSTALL_TARGETS_DEFAULT_ARGS})
