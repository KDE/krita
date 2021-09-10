project( lcmsengine )

add_subdirectory(tests)

include_directories(SYSTEM
    ${LCMS2_INCLUDE_DIRS}
)

include_directories( 
    ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/cmyk_u8
    ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/cmyk_u16
    ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/cmyk_f32
    ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/gray_u8
    ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/gray_u16
    ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/gray_f32
    ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/lab_u8
    ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/lab_u16
    ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/lab_f32
    ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/rgb_u8
    ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/rgb_u16
    ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/rgb_f32
    ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/xyz_u8
    ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/xyz_u16
    ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/xyz_f32
    ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/ycbcr_u8
    ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/ycbcr_u16
    ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/ycbcr_f32
    ${CMAKE_CURRENT_SOURCE_DIR}/colorprofiles
)

if (HAVE_LCMS24 AND OpenEXR_FOUND)
include_directories( 
    ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/gray_f16
    ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/rgb_f16
    ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/xyz_f16
)
endif ()

set(LINK_OPENEXR_LIB)
if(OpenEXR_FOUND)
    set(LINK_OPENEXR_LIB ${OPENEXR_LIBRARIES})
endif()


set ( lcmsengine_SRCS
    colorspaces/cmyk_u8/CmykU8ColorSpace.cpp
    colorspaces/cmyk_u16/CmykU16ColorSpace.cpp
    colorspaces/cmyk_f32/CmykF32ColorSpace.cpp

    colorspaces/gray_u8/GrayU8ColorSpace.cpp
    colorspaces/gray_u16/GrayU16ColorSpace.cpp
    colorspaces/gray_f32/GrayF32ColorSpace.cpp

    colorspaces/lab_u8/LabU8ColorSpace.cpp
    colorspaces/lab_u16/LabColorSpace.cpp
    colorspaces/lab_f32/LabF32ColorSpace.cpp

    colorspaces/xyz_u8/XyzU8ColorSpace.cpp
    colorspaces/xyz_u16/XyzU16ColorSpace.cpp
    colorspaces/xyz_f32/XyzF32ColorSpace.cpp

    colorspaces/rgb_u8/RgbU8ColorSpace.cpp
    colorspaces/rgb_u16/RgbU16ColorSpace.cpp
    colorspaces/rgb_f32/RgbF32ColorSpace.cpp

    colorspaces/ycbcr_u8/YCbCrU8ColorSpace.cpp
    colorspaces/ycbcr_u16/YCbCrU16ColorSpace.cpp
    colorspaces/ycbcr_f32/YCbCrF32ColorSpace.cpp

    colorprofiles/LcmsColorProfileContainer.cpp
    colorprofiles/IccColorProfile.cpp
    IccColorSpaceEngine.cpp
    LcmsColorSpace.cpp
    LcmsEnginePlugin.cpp
)

if (HAVE_LCMS24 AND OpenEXR_FOUND)
    set ( lcmsengine_SRCS
        ${lcmsengine_SRCS}
        colorspaces/gray_f16/GrayF16ColorSpace.cpp
        colorspaces/rgb_f16/RgbF16ColorSpace.cpp
        colorspaces/xyz_f16/XyzF16ColorSpace.cpp
    )
endif ()



add_library(kritalcmsengine MODULE ${lcmsengine_SRCS})

target_link_libraries(kritalcmsengine kritapigment kritawidgetutils KF5::I18n KF5::CoreAddons ${LCMS2_LIBRARIES}  ${LINK_OPENEXR_LIB})
install(TARGETS kritalcmsengine DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

