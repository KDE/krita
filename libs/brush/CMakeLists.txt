add_subdirectory( tests )

include_directories(SYSTEM
    ${EIGEN3_INCLUDE_DIR}
)

set(kritalibbrush_LIB_SRCS
    kis_predefined_brush_factory.cpp
    kis_auto_brush.cpp
    kis_boundary.cc
    kis_brush.cpp
    kis_scaling_size_brush.cpp
    kis_brush_registry.cpp
    kis_brush_server.cpp
    kis_gbr_brush.cpp
    kis_abr_brush.cpp
    kis_abr_brush_collection.cpp
    kis_imagepipe_brush.cpp
    kis_pipebrush_parasite.cpp
    kis_png_brush.cpp
    kis_svg_brush.cpp
    kis_qimage_pyramid.cpp
    KisSharedQImagePyramid.cpp
    kis_text_brush.cpp
    kis_auto_brush_factory.cpp
    kis_text_brush_factory.cpp
)

kis_add_library(kritalibbrush SHARED ${kritalibbrush_LIB_SRCS} )
generate_export_header(kritalibbrush BASE_NAME kritabrush EXPORT_MACRO_NAME BRUSH_EXPORT)

if (WIN32)
    target_link_libraries(kritalibbrush kritaimage Qt5::Svg ${WIN32_PLATFORM_NET_LIBS})
else ()
    target_link_libraries(kritalibbrush kritaimage Qt5::Svg)
endif ()


if(HAVE_VC)
  include_directories(SYSTEM ${Vc_INCLUDE_DIR})
  target_link_libraries(kritalibbrush  ${Vc_LIBRARIES})
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Vc_DEFINITIONS}")
endif()

set_target_properties(kritalibbrush PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)

install(TARGETS kritalibbrush  ${INSTALL_TARGETS_DEFAULT_ARGS})

