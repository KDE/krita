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
    KoColorTransformation.cpp
    KoColorTransformationFactory.cpp
    KoColorTransformationFactoryRegistry.cpp
    KoCompositeOp.cpp
    KoCopyColorConversionTransformation.cpp
    KoFallBackColorTransformation.cpp
    KoHistogramProducer.cpp
    KoIccColorSpaceEngine.cpp
    KoLcmsColorSpace.cpp
    KoMultipleColorConversionTransformation.cpp
    KoUniqueNumberForIdServer.cpp
    colorprofiles/KoIccColorProfile.cpp
    colorprofiles/KoLcmsColorProfileContainer.cpp
    colorspaces/KoAlphaColorSpace.cpp
    colorspaces/KoLabColorSpace.cpp
    colorspaces/KoRgbU16ColorSpace.cpp
    colorspaces/KoRgbU8ColorSpace.cc
    colorspaces/KoRgbU8CompositeOp.cpp
)

add_subdirectory(tests)


kde4_add_library(pigmentcms SHARED ${pigmentcms_SRCS})

target_link_libraries(
    pigmentcms
    komain
    ${LCMS_LIBRARIES}
    ${KDE4_KPARTS_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${X11_X11_LIB}
    ${LINK_OPENEXR_LIB}
    )

target_link_libraries(
    pigmentcms LINK_INTERFACE_LIBRARIES komain ${LCMS_LIBRARIES} ${LINK_OPENEXR_LIB}
)

set_target_properties(pigmentcms PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)

install(TARGETS pigmentcms  ${INSTALL_TARGETS_DEFAULT_ARGS})

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
         KoMixColorsOpImpl.h
         KoHistogramProducer.h
         DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

install( FILES  pigment.desktop pigmentextension.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

