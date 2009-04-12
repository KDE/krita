add_subdirectory( bigbrother )

add_subdirectory( bracketing2hdr )
add_subdirectory( colorrange )
add_subdirectory( colorspaceconversion )
add_subdirectory( compose )
add_subdirectory( dropshadow )
add_subdirectory( histogram )
add_subdirectory( imagesize )
add_subdirectory( metadataeditor )
add_subdirectory( modify_selection )
add_subdirectory( rotateimage )
add_subdirectory( scripting ) # TODO: port it to the change of KisFilter, KisPaintOpPreset
add_subdirectory( separate_channels )
add_subdirectory( shearimage )
add_subdirectory( smallcolorselector )
add_subdirectory( specificcolorselector )
add_subdirectory( together )
add_subdirectory( tonemapping )
add_subdirectory( trianglecolorselector )
add_subdirectory( histogram_docker )

macro_optional_find_package(GMM)
macro_log_feature(GMM_FOUND "Gmm++" "Generic C++ template library for sparse, dense and skyline matrices" "http://home.gna.org/getfem/download.html" FALSE "" "Required by the Krita panorama plugin")



if(GMM_FOUND AND NOT MSVC)
    add_subdirectory( panorama )
endif(GMM_FOUND AND NOT MSVC)

if(HAVE_OPENGL AND HAVE_GLEW)
    add_subdirectory( glsl )
endif(HAVE_OPENGL AND HAVE_GLEW)

if(OPENCTL_FOUND)
  add_subdirectory( painterlyframework )
endif(OPENCTL_FOUND)

