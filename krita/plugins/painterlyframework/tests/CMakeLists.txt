include_directories(  ${KOMAIN_INCLUDES} ${KOGUIUTILS_INCLUDES} "../" )

########### next target ###############

set(kis_complex_color_test_SRCS
	kis_complex_color_test.cpp
	../kis_rgbks_colorspace.cpp
	../mathematics.cpp)

kde4_add_unit_test(KisComplexColorTest TESTNAME krita-plugins-KisComplexColorTest  ${kis_complex_color_test_SRCS})

target_link_libraries(KisComplexColorTest  ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage kritaui glpk ${QT_QTTEST_LIBRARY})
