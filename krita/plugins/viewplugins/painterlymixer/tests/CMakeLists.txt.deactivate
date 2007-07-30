include_directories(  ${KOMAIN_INCLUDES} ${KOGUIUTILS_INCLUDES} ".." )


set(simplex_test_SRCS simplex_test.cpp
    ../kis_illuminant_profile.cpp
	../kis_reflectance_colorspace.cpp
	../kis_ks_colorspace.cpp)

kde4_add_unit_test(KisSimplexTester TESTNAME krita-painterlymixer-KisSimplexTester  ${simplex_test_SRCS})

target_link_libraries(KisSimplexTester  glpk ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})


########### next target ###############

set(illuminant_test_SRCS illuminant_test.cpp
    ../kis_illuminant_profile.cpp
	../kis_reflectance_colorspace.cpp
	../kis_ks_colorspace.cpp)

kde4_add_unit_test(KisIlluminantTester TESTNAME krita-painterlymixer-KisIlluminantTester  ${illuminant_test_SRCS})

target_link_libraries(KisIlluminantTester glpk ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})


########### next target ###############

set(reflectance_test_SRCS reflectance_test.cpp
    ../kis_illuminant_profile.cpp
	../kis_reflectance_colorspace.cpp
	../kis_ks_colorspace.cpp)

kde4_add_unit_test(KisReflectanceTester TESTNAME krita-painterlymixer-KisReflectanceTester  ${reflectance_test_SRCS})

target_link_libraries(KisReflectanceTester glpk ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})


########### next target ###############

set(plot_test_SRCS plot_test.cpp
    ../kis_illuminant_profile.cpp
	../kis_reflectance_colorspace.cpp
	../kis_ks_colorspace.cpp)

kde4_add_unit_test(KisPlotTester TESTNAME krita-painterlymixer-KisPlotTester  ${plot_test_SRCS})

target_link_libraries(KisPlotTester glpk ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})
