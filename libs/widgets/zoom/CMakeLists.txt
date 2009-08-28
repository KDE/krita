add_subdirectory( tests )
include_directories(${PIGMENT_INCLUDES} ${FLAKE_INCLUDES} ${CMAKE_SOURCE_DIR}/libs/widgets)

########### next target ###############

set(kozoomaction_LIB_SRCS
    KoZoomAction.cpp
    KoZoomController.cpp
    KoZoomInput.cpp
)

kde4_add_library(kozoomaction SHARED ${kozoomaction_LIB_SRCS})

target_link_libraries(kozoomaction flake)
target_link_libraries(kozoomaction LINK_INTERFACE_LIBRARIES flake)

set_target_properties(kozoomaction PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)
install(TARGETS kozoomaction  ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES zoom-width.png zoom-page.png zoom-pixels.png DESTINATION ${DATA_INSTALL_DIR}/koffice/icons/hicolor/22x22/actions)

########### install files ###############
install( FILES
    KoZoomAction.h
    KoZoomController.h
    KoZoomInput.h
    DESTINATION
    ${INCLUDE_INSTALL_DIR}
COMPONENT Devel)
