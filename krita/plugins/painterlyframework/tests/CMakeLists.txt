set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${KOMAIN_INCLUDES} "../" )

########### KisIlluminantProfile Test ###############

set(kis_illuminant_profile_test_SRCS
    kis_illuminant_profile_test.cpp
    ../ks_colorspaces.cpp
    ../channel_converter.cpp
    ../kis_illuminant_profile.cpp
    ../kis_ksqp_colorspace.cpp
    ../cqp/cqpminimizer.c
    ../cqp/initial_point.c
    ../cqp/mg_pdip.c)

kde4_add_unit_test(KisIlluminantProfileTest
                   TESTNAME krita-plugins-KisIlluminantProfileTest
                   ${kis_illuminant_profile_test_SRCS})

target_link_libraries(KisIlluminantProfileTest pigmentcms ${KDE4_KDEUI_LIBS} ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES} ${QT_QTTEST_LIBRARY})

########### ChannelConverter Test ###############

set(channel_converter_test_SRCS
    channel_converter_test.cpp
    ../ks_colorspaces.cpp
    ../channel_converter.cpp
    ../kis_illuminant_profile.cpp
    ../kis_ksqp_colorspace.cpp
    ../cqp/cqpminimizer.c
    ../cqp/initial_point.c
    ../cqp/mg_pdip.c)

kde4_add_unit_test(ChannelConverterTest
                   TESTNAME krita-plugins-ChannelConverterTest
                   ${channel_converter_test_SRCS})

target_link_libraries(ChannelConverterTest pigmentcms ${KDE4_KDEUI_LIBS} ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES} ${QT_QTTEST_LIBRARY})

########### KisKSColorSpace Test ###############

set(kis_ks_colorspace_test_SRCS
    kis_ks_colorspace_test.cpp
    ../ks_colorspaces.cpp
    ../channel_converter.cpp
    ../kis_illuminant_profile.cpp
    ../kis_ksqp_colorspace.cpp
    ../cqp/cqpminimizer.c
    ../cqp/initial_point.c
    ../cqp/mg_pdip.c)

kde4_add_unit_test(KisKSColorSpaceTest
                   TESTNAME krita-plugins-KisKSColorSpaceTest
                   ${kis_ks_colorspace_test_SRCS})

target_link_libraries(KisKSColorSpaceTest pigmentcms ${KDE4_KDEUI_LIBS} ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES} ${QT_QTTEST_LIBRARY})

########### Mixing Test ###############

set(mixing_test_SRCS
    mixing_test.cpp
    ../ks_colorspaces.cpp
    ../channel_converter.cpp
    ../kis_illuminant_profile.cpp
    ../kis_ksqp_colorspace.cpp
    ../cqp/cqpminimizer.c
    ../cqp/initial_point.c
    ../cqp/mg_pdip.c)

kde4_add_unit_test(MixingTest
                   TESTNAME krita-plugins-MixingTest
                   ${mixing_test_SRCS})

target_link_libraries(MixingTest pigmentcms ${KDE4_KDEUI_LIBS} ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES} ${QT_QTTEST_LIBRARY})
