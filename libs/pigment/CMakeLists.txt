include_directories( ${KOMAIN_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${LCMS_INCLUDE_DIR} )

set(FILE_OPENEXR_SOURCES)
set(LINK_OPENEXR_LIB)
if(OPENEXR_FOUND)
	include_directories(${OPENEXR_INCLUDE_DIR})
	set(LINK_OPENEXR_LIB ${OPENEXR_LIBRARIES})
endif(OPENEXR_FOUND)

set(pigment_SRCS
    KoColorSpaceRegistry.cpp
    KoColor.cpp
    KoColorSpace.cpp
    KoColorSpaceMaths.cpp
    KoCompositeOp.cpp
    KoColorProfile.cpp
    KoColorConversions.cpp
    KoBasicHistogramProducers.cpp
    KoHistogramProducer.cpp
    KoColorConvertionTransformation.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/colorprofiles/KoIccColorProfile.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/colorprofiles/KoLcmsColorProfile.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/colorprofiles/KoLcmsRGBColorProfile.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/KoAlphaColorSpace.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/KoLabColorSpace.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/KoRgbU8CompositeOp.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/KoRgbU8ColorSpace.cc
   ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/KoRgbU16ColorSpace.cpp
)

add_subdirectory(tests)


kde4_add_library(pigment SHARED ${pigment_SRCS})

target_link_libraries(pigment ${KDE4_KPARTS_LIBS} ${LCMS_LIBRARIES} ${LINK_OPENEXR_LIB} komain)

set_target_properties(pigment PROPERTIES VERSION 1.0.0 SOVERSION 1 )


install(TARGETS pigment  DESTINATION ${LIB_INSTALL_DIR})

install( FILES  KoColorSpaceRegistry.h KoChannelInfo.h      KoColor.h      KoColorSpace.h      KoCompositeOp.h      KoColorProfile.h  KoBasicHistogramProducers.h KoBasicHistogramProducers.h DESTINATION ${INCLUDE_INSTALL_DIR})
install( FILES  pigment.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})

