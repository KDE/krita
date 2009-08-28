add_subdirectory( tests )

#add_definitions(-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

set(kritalibbrush_LIB_SRCS
    kis_auto_brush.cpp
    kis_boundary.cc
    kis_brush.cpp
    kis_brush_registry.cpp
    kis_brush_server.cpp
    kis_gbr_brush.cpp
    kis_imagepipe_brush.cpp
    kis_imagepipe_brush_p.cpp
    kis_qimage_mask.cpp
    kis_scaled_brush.cpp
    kis_text_brush.cpp
    kis_auto_brush_factory.cpp
    kis_text_brush_factory.cpp
    kis_gbr_brush_factory.cpp
)

kde4_add_library(kritalibbrush SHARED ${kritalibbrush_LIB_SRCS} )

target_link_libraries(kritalibbrush kritaui )
target_link_libraries(kritalibbrush LINK_INTERFACE_LIBRARIES kritaui)

set_target_properties(kritalibbrush PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)

install(TARGETS kritalibbrush  ${INSTALL_TARGETS_DEFAULT_ARGS})

