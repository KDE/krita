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

if(MSVC)
  # avoid "cannot open file 'LIBC.lib'" error
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:LIBC.LIB")
endif(MSVC)

message(STATUS ${OPENCTL_FOUND})

if(OPENCTL_FOUND)
  include_directories(${OPENCTL_INCLUDE_DIR})
  set(LINK_OPENCTL_LIB ${OPENCTL_LIBRARIES})
  add_definitions(${OPENCTL_DEFINITIONS})
  add_subdirectory(ctlmodules)
  set( OPENCTL_PIGMENT_SRCS
    colorprofiles/KoCtlColorProfile.cpp
    KoCtlColorConversionTransformation.cpp
    KoCtlColorSpace.cpp
  )
endif(OPENCTL_FOUND)

set(pigmentcms_SRCS
    KoBasicHistogramProducers.cpp
    KoColor.cpp
    KoColorConversionAlphaTransformation.cpp
    KoColorConversionCache.cpp
    KoColorConversions.cpp
    KoColorConversionSystem.cpp
    KoColorConversionTransformation.cpp
    KoColorConversionTransformationFactory.cpp
    KoColorModelStandardIds.cpp
    KoColorProfile.cpp
    KoColorSpace.cpp
    KoColorSpaceEngine.cpp
    KoColorSpaceMaths.cpp
    KoColorSpaceRegistry.cpp
    KoColorTransformationFactory.cpp
    KoColorTransformationFactoryRegistry.cpp
    KoCompositeOp.cpp
    KoCopyColorConversionTransformation.cpp
    KoFallBackColorTransformation.cpp
    KoHistogramProducer.cpp
    KoIccColorSpaceEngine.cpp
    KoLcmsColorSpace.cpp
    KoMultipleColorConversionTransformation.cpp
    colorprofiles/KoIccColorProfile.cpp
    colorprofiles/KoHdrColorProfile.cpp
    colorprofiles/KoLcmsColorProfileContainer.cpp
    colorspaces/KoAlphaColorSpace.cpp
    colorspaces/KoLabColorSpace.cpp
    colorspaces/KoRgbU16ColorSpace.cpp
    colorspaces/KoRgbU8ColorSpace.cc
    colorspaces/KoRgbU8CompositeOp.cpp
    ${OPENCTL_PIGMENT_SRCS}
)

add_subdirectory(tests)


kde4_add_library(pigmentcms SHARED ${pigmentcms_SRCS})

target_link_libraries(pigmentcms ${KDE4_KPARTS_LIBS} ${LCMS_LIBRARIES} ${LINK_OPENEXR_LIB} komain ${LINK_OPENCTL_LIB})

set_target_properties(pigmentcms PROPERTIES VERSION 1.0.0 SOVERSION 1 )


install(TARGETS pigmentcms  DESTINATION ${LIB_INSTALL_DIR})

install( FILES 
         pigment_export.h
         DebugPigment.h
         KoBasicHistogramProducers.h
         KoChannelInfo.h
         KoColor.h
         KoColorConversionTransformation.h
         KoColorConversionTransformationAbstractFactory.h
         KoColorConversionTransformationFactory.h
         KoColorModelStandardIds.h
         KoColorProfile.h
         KoColorSpace.h
         KoColorSpaceEngine.h
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

