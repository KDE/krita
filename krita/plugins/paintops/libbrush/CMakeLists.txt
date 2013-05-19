add_subdirectory( tests )

set(kritalibbrush_LIB_SRCS
    kis_auto_brush.cpp
    kis_boundary.cc
    kis_brush.cpp
    kis_brush_registry.cpp
    kis_brush_server.cpp
    kis_gbr_brush.cpp
    kis_abr_brush.cpp
    kis_abr_brush_collection.cpp
    kis_imagepipe_brush.cpp
    kis_imagepipe_brush_p.cpp
    kis_png_brush.cpp
    kis_png_brush_factory.cpp
    kis_svg_brush.cpp
    kis_svg_brush_factory.cpp
    kis_qimage_mask.cpp
    kis_scaled_brush.cpp
    kis_text_brush.cpp
    kis_auto_brush_factory.cpp
    kis_text_brush_factory.cpp
    kis_gbr_brush_factory.cpp
    kis_abr_brush_factory.cpp
)

kde4_add_library(kritalibbrush SHARED ${kritalibbrush_LIB_SRCS} )

target_link_libraries(kritalibbrush kritaui)

if(HAVE_VC)
  include_directories(${Vc_INCLUDE_DIR})
  target_link_libraries(kritalibbrush  ${Vc_LIBRARIES})
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Vc_DEFINITIONS}")
endif(HAVE_VC)

if (WIN32)
target_link_libraries(kritalibbrush LINK_INTERFACE_LIBRARIES kritaui ${WIN32_PLATFORM_NET_LIBS})
else (WIN32)
target_link_libraries(kritalibbrush LINK_INTERFACE_LIBRARIES kritaui)
endif (WIN32)

set_target_properties(kritalibbrush PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)

install(TARGETS kritalibbrush  ${INSTALL_TARGETS_DEFAULT_ARGS})

