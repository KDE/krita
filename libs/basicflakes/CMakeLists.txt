add_subdirectory(plugin)

include_directories(${KOTEXT_INCLUDES} ${KOODF_INCLUDES} ${PIGMENT_INCLUDES} ${KOMAIN_INCLUDES})

set(basicflakes_LIB_SRCS
    tools/KoCreatePathTool.cpp
    tools/KoPencilTool.cpp
)

kde4_add_ui_files( basicflakes_LIB_SRCS
)


kde4_add_library(basicflakes SHARED ${basicflakes_LIB_SRCS})

target_link_libraries(basicflakes kowidgets flake pigmentcms)

set_target_properties(basicflakes PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)

install(TARGETS basicflakes  ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
    tools/KoCreatePathTool.h
    tools/KoPencilTool.h
    basicflakes_export.h

    DESTINATION
    ${INCLUDE_INSTALL_DIR}/calligra
COMPONENT Devel)
