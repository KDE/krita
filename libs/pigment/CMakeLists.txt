add_subdirectory( colorprofiles )

project(pigmentcms)

include_directories( ${KOMAIN_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${LCMS_INCLUDE_DIR} )

set(FILE_OPENEXR_SOURCES)
set(LINK_OPENEXR_LIB)
if(OPENEXR_FOUND)
	include_directories(${OPENEXR_INCLUDE_DIR})
	set(LINK_OPENEXR_LIB ${OPENEXR_LIBRARIES})
    add_definitions(${OPENEXR_DEFINITIONS})
endif(OPENEXR_FOUND)

add_definitions("-Dand=\\&\\& -Dor=\\|\\| -Dnot=\\!")

message(STATUS ${OPENCTL_FOUND})

if(OPENCTL_FOUND)
  include_directories(${OPENCTL_INCLUDE_DIR})
  set(LINK_OPENCTL_LIB ${OPENCTL_LIBRARIES})
  add_definitions(${OPENCTL_DEFINITIONS})
  set( OPENCTL_PIGMENT_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/colorprofiles/KoCtlColorProfile.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/KoCtlColorSpace.cpp
  )
endif(OPENCTL_FOUND)

set(pigmentcms_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/KoBasicHistogramProducers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/KoColor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/KoColorConversionAlphaTransformation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/KoColorConversions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/KoColorConversionSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/KoColorConversionTransformation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/KoColorConversionTransformationFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/KoColorModelStandardIds.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/KoColorProfile.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/KoColorSpace.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/KoColorSpaceMaths.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/KoColorSpaceRegistry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/KoColorTransformationFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/KoColorTransformationFactoryRegistry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/KoCompositeOp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/KoCopyColorConversionTransformation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/KoFallBackColorTransformation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/KoHistogramProducer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/KoLcmsColorSpace.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/KoColorConversionCache.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/KoMultipleColorConversionTransformation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/colorprofiles/KoIccColorProfile.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/colorprofiles/KoHdrColorProfile.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/colorprofiles/KoLcmsColorProfileContainer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/KoAlphaColorSpace.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/KoLabColorSpace.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/KoRgbU16ColorSpace.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/KoRgbU8ColorSpace.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/KoRgbU8CompositeOp.cpp
    ${OPENCTL_PIGMENT_SRCS}
)

add_subdirectory(tests)


kde4_add_library(pigmentcms SHARED ${pigmentcms_SRCS})

target_link_libraries(pigmentcms ${KDE4_KPARTS_LIBS} ${LCMS_LIBRARIES} ${LINK_OPENEXR_LIB} komain ${LINK_OPENCTL_LIB})

set_target_properties(pigmentcms PROPERTIES VERSION 1.0.0 SOVERSION 1 )


install(TARGETS pigmentcms  DESTINATION ${LIB_INSTALL_DIR})

install( FILES 
         pigment_export.h
         KoBasicHistogramProducers.h
         KoChannelInfo.h
         KoColor.h
         KoColorConversionTransformation.h
         KoColorConversionTransformationFactory.h
         KoColorModelStandardIds.h
         KoColorProfile.h
         KoColorSpace.h
         KoColorSpaceAbstract.h
         KoColorSpaceConstants.h
         KoColorSpaceMaths.h
         KoColorSpaceRegistry.h
         KoColorSpaceTraits.h
         KoColorTransformation.h
         KoColorTransformationFactory.h
         KoColorTransformationFactoryRegistry.h
         KoCompositeOp.h
         KoFallBackColorTransformation.h
         KoIntegerMaths.h
         KoLabDarkenColorTransformation.h
         KoHistogramProducer.h
         DESTINATION ${INCLUDE_INSTALL_DIR})

install( FILES  pigment.desktop pigmentextension.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

