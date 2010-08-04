project( lcmsengine )

add_subdirectory(tests)
add_subdirectory(colorprofiles)

include_directories( ${PIGMENT_INCLUDES} ${LCMS2_INCLUDE_DIR} )

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
    colorspaces/cmyk_u8/CmykU8ColorSpace.cpp
    colorspaces/cmyk_u16/CmykU16ColorSpace.cpp
    colorspaces/gray_u8/GrayU8ColorSpace.cpp
    colorspaces/lab_u16/LabColorSpace.cpp
    colorspaces/xyz_u16/XyzU16ColorSpace.cpp
    colorspaces/gray_u8_no_alpha/GrayU8NoAlphaColorSpace.cpp
    colorspaces/gray_u16_no_alpha/GrayU16NoAlphaColorSpace.cpp
    colorspaces/rgb_u8/RgbU8ColorSpace.cpp
    colorspaces/gray_u16/GrayU16ColorSpace.cpp
    colorspaces/rgb_u16/RgbU16ColorSpace.cpp
    colorprofiles/LcmsColorProfileContainer.cpp
    colorprofiles/IccColorProfile.cpp
    IccColorSpaceEngine.cpp
    LcmsColorSpace.cpp
    LcmsEnginePlugin.cpp
)

kde4_add_plugin(kolcmsengine ${lcmsengine_SRCS})

target_link_libraries(kolcmsengine pigmentcms ${LCMS2_LIBRARIES} ${KDE4_KPARTS_LIBS} ${LINK_OPENEXR_LIB})
install(TARGETS kolcmsengine DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES kolcmsengine.desktop DESTINATION ${SERVICES_INSTALL_DIR})

