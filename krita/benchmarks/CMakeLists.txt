set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(  ${KOMAIN_INCLUDES}  ${CMAKE_SOURCE_DIR}/krita/image/metadata ${CMAKE_SOURCE_DIR}/krita/sdk/tests )

########### next target ###############

#set(kis_cs_conversion_test_SRCS kis_cs_conversion_test.cpp )
#kde4_add_unit_test(KisCsConversionTest TESTNAME krita-image-KisCsConversionTest ${kis_cs_conversion_test_SRCS})
#target_link_libraries(KisCsConversionTest ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

