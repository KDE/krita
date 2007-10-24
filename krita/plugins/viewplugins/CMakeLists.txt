#add_subdirectory( performancetest )
add_subdirectory( colorrange )
add_subdirectory( compose )
add_subdirectory( colorspaceconversion )
add_subdirectory( dropshadow )
add_subdirectory( filtersgallery )
add_subdirectory( histogram )
#add_subdirectory( histogram_docker )
MESSAGE("Krita: port the histogram docker")
add_subdirectory( imagesize )
add_subdirectory( modify_selection )
add_subdirectory( rotateimage )
add_subdirectory( separate_channels )
add_subdirectory( shearimage )
add_subdirectory( together )
# add_subdirectory( scripting ) # TODO: port it to the change of KisFilter
add_subdirectory( bracketing2hdr )
add_subdirectory( metadataeditor )
add_subdirectory( bigbrother )
add_subdirectory( tonemapping )

find_package(GMM)

macro_log_feature(EIGEN_FOUND "Eigen" "The Krita panorama plugin will not be built" "http://eigen.tuxfamily.org" FALSE "")
macro_log_feature(GMM_FOUND "GMM" "The Krita panorama plugin will not be built" "Available from kdesupport" FALSE "")

if(EIGEN_FOUND AND GMM_FOUND)
    add_subdirectory( panorama )
endif(EIGEN_FOUND AND GMM_FOUND)

macro_optional_find_package(GLUT)

macro_log_feature(HAVE_OPENGL "OpenGL" "The Krita shader filter plugin will not be built" "" FALSE "")
macro_log_feature(HAVE_GLEW "GLEW" "The Krita shader filter plugin will not be built" "http://glew.sourceforge.net" FALSE "")
macro_log_feature(GLUT_FOUND "GLUT" "The Krita shader filter plugin will not be built" "http://www.opengl.org/resources/libraries/glut" FALSE "")

if(HAVE_OPENGL)
    IF(HAVE_GLEW)
        if (GLUT_FOUND)
           add_subdirectory( glsl )
        endif(GLUT_FOUND)
    ENDIF(HAVE_GLEW)
endif(HAVE_OPENGL)

