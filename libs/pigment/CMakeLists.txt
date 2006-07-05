include_directories( ${KOFFICECORE_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${LCMS_INCLUDE_DIR} )

find_package(LCMS)

set(FILE_OPENEXR_SOURCES)
set(LINK_OPENEXR_LIB)
if(OPENEXR_FOUND)
	set(FILE_OPENEXR_SOURCES KoF16HalfColorSpaceTrait.cpp) 
	include_directories(${OPENEXR_INCLUDE_DIR})
	set(LINK_OPENEXR_LIB ${OPENEXR_LIBRARIES})
endif(OPENEXR_FOUND)

set(pigment_SRCS
    KoColorSpaceFactoryRegistry.cpp
    KoColor.cpp
    KoColorSpace.cpp
    KoCompositeOp.cpp
    KoColorProfile.cpp
    KoLcmsColorSpaceTrait.cpp
    KoU8ColorSpaceTrait.cpp
    KoU16ColorSpaceTrait.cpp
    KoF32HalfColorSpaceTrait.cpp
	${FILE_OPENEXR_SOURCES}
   ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/KoAlphaColorSpace.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/KoLabColorSpace.cpp
)

kde4_automoc(${pigment_SRCS})

kde4_add_library(pigment SHARED ${pigment_SRCS})

target_link_libraries(pigment ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} ${LCMS_LIBRARIES} ${LINK_OPENEXR_LIB})

set_target_properties(pigment PROPERTIES VERSION 1.0.0 SOVERSION 1 )


install(TARGETS pigment  DESTINATION ${LIB_INSTALL_DIR})

install( FILES  KoColorSpaceFactoryRegistry.h KoChannelInfo.h      KoColor.h      KoColorSpace.h      KoCompositeOp.h      KoColorProfile.h      KoU8ColorSpaceTrait.h KoU16ColorSpaceTrait.h KoF16HalfColorSpaceTrait.h KoF32HalfColorSpaceTrait.h  DESTINATION ${INCLUDE_INSTALL_DIR})


