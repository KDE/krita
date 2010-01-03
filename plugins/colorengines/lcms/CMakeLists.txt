project( lcmsengine )

include_directories(  ${PIGMENT_INCLUDES} ${LCMS_INCLUDE_DIR} )

SET ( lcmsengine_SRCS
    colorspaces/cmyk_u8/KoCmykU8ColorSpace.cpp
    colorspaces/cmyk_u16/KoCmykU16ColorSpace.cpp
    colorspaces/gray_u8/tests/grayscale_tester.cpp
    colorspaces/gray_u8/KoGrayU8ColorSpace.cpp
    colorspaces/lab_u16/KoLabColorSpace.cpp
    colorspaces/xyz_u16/KoXyzU16ColorSpace.cpp
    colorspaces/gray_u8_no_alpha/KoGrayU8NoAlphaColorSpace.cpp
    colorspaces/gray_u8_no_alpha/tests/grayscale_tester.cpp
    colorspaces/gray_u16_no_alpha/KoGrayU16NoAlphaColorSpace.cpp
    colorspaces/rgb_u8/KoRgbU8CompositeOp.cpp
    colorspaces/rgb_u8/KoRgbU8ColorSpace.cpp
    colorspaces/gray_u16/KoGrayU16ColorSpace.cpp
    colorspaces/rgb_u16/KoRgbU16ColorSpace.cpp
)

kde4_add_plugin(kolcmsengine ${lcmsengine_SRCS})
    
target_link_libraries(kolcmsengine pigmentcms)
install(TARGETS kolcmsengine DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES kolcmsengine.desktop DESTINATION ${SERVICES_INSTALL_DIR})
