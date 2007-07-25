include_directories(  ${KOMAIN_INCLUDES} ${KOGUIUTILS_INCLUDES} ".." )


########### next target ###############

set(illuminant_test_SRCS illuminant_test.cpp
    ../kis_illuminant_profile.cpp
	../kis_ks_colorspace.cpp)

kde4_add_unit_test(KisIlluminantTester TESTNAME krita-painterlymixer-KisIlluminantTester  ${illuminant_test_SRCS})

target_link_libraries(KisIlluminantTester  glpk ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})
