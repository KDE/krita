set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(  ${KOMAIN_INCLUDES} ${KOGUIUTILS_INCLUDES} ${PIGMENT_INCLUDES} )

########### next target ###############

set(KisRgbFloatHDRColorSpaceTest_SRCS kis_rgb_float_hdr_colorspace_test.cpp )

kde4_add_unit_test(KisRgbFloatHDRColorSpaceTest TESTNAME krita-colorspaces-KisRgbFloatHDRColorSpaceTest ${KisRgbFloatHDRColorSpaceTest_SRCS})

if(OPENEXR_FOUND)
    set(rgbf16_lib kritargbf16hdr)
endif(OPENEXR_FOUND)

target_link_libraries(KisRgbFloatHDRColorSpaceTest  ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage kritargbf32hdr ${rgbf16_lib} ${QT_QTTEST_LIBRARY})

