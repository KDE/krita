project( lcmsengine )

add_subdirectory(tests)
add_subdirectory(colorprofiles)
include_directories( ${PIGMENT_INCLUDES} ${LCMS_INCLUDE_DIR} )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/cmyk_u16
    ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/cmyk_u8
    ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/gray_u16
    ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/gray_u16_no_alpha
    ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/gray_u8
    ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/gray_u8_no_alpha
    ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/lab_u16
    ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/rgb_u16
    ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/rgb_u8
    ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/xyz_u16
    ${CMAKE_CURRENT_SOURCE_DIR}/colorprofiles
)

set(FILE_OPENEXR_SOURCES)
set(LINK_OPENEXR_LIB)
if(OPENEXR_FOUND)
    include_directories(${OPENEXR_INCLUDE_DIR})
    set(LINK_OPENEXR_LIB ${OPENEXR_LIBRARIES})
    add_definitions(${OPENEXR_DEFINITIONS})
endif(OPENEXR_FOUND)


SET ( lcmsengine_SRCS
    colorspaces/cmyk_u8/KoCmykU8ColorSpace.cpp
    colorspaces/cmyk_u16/KoCmykU16ColorSpace.cpp
    colorspaces/gray_u8/KoGrayU8ColorSpace.cpp
    colorspaces/lab_u16/KoLabColorSpace.cpp
    colorspaces/xyz_u16/KoXyzU16ColorSpace.cpp
    colorspaces/gray_u8_no_alpha/KoGrayU8NoAlphaColorSpace.cpp
    colorspaces/gray_u16_no_alpha/KoGrayU16NoAlphaColorSpace.cpp
    colorspaces/rgb_u8/KoRgbU8CompositeOp.cpp
    colorspaces/rgb_u8/KoRgbU8ColorSpace.cpp
    colorspaces/gray_u16/KoGrayU16ColorSpace.cpp
    colorspaces/rgb_u16/KoRgbU16ColorSpace.cpp
    colorprofiles/KoLcmsColorProfileContainer.cpp
    colorprofiles/KoIccColorProfile.cpp
    KoIccColorSpaceEngine.cpp
    KoLcmsColorSpace.cpp
    KoLcmsEnginePlugin.cpp
)

kde4_add_plugin(kolcmsengine ${lcmsengine_SRCS})

target_link_libraries(kolcmsengine pigmentcms ${LCMS_LIBRARIES} ${KDE4_KPARTS_LIBS} ${LINK_OPENEXR_LIB})
install(TARGETS kolcmsengine DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES kolcmsengine.desktop DESTINATION ${SERVICES_INSTALL_DIR})

