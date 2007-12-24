set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${KOMAIN_INCLUDES} "../" )

########### KisIlluminantProfile Test ###############

set(kis_illuminant_profile_test_SRCS
    kis_illuminant_profile_test.cpp
    ../kis_illuminant_profile.cpp)

kde4_add_unit_test(KisIlluminantProfileTest
                   TESTNAME krita-plugins-KisIlluminantProfileTest
                   ${kis_illuminant_profile_test_SRCS})

target_link_libraries(KisIlluminantProfileTest
                      ${KDE4_KDEUI_LIBS} komain kritaimage ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES} ${QT_QTTEST_LIBRARY})

########### ChannelConverter Test ###############

set(channel_converter_test_SRCS
    channel_converter_test.cpp
    ../channel_converter.cpp)

kde4_add_unit_test(ChannelConverterTest
                   TESTNAME krita-plugins-ChannelConverterTest
                   ${channel_converter_test_SRCS})

target_link_libraries(ChannelConverterTest
                      ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})

########### KisKS3ColorSpace Test ###############

set(kis_ks3_colorspace_test_SRCS
    kis_ks3_colorspace_test.cpp
    ../kis_illuminant_profile.cpp
    ../channel_converter.cpp
    ../kis_ks3_colorspace.cpp)

kde4_add_unit_test(KisKS3ColorSpaceTest
                   TESTNAME krita-plugins-KisKS3ColorSpaceTest
                   ${kis_ks3_colorspace_test_SRCS})

target_link_libraries(KisKS3ColorSpaceTest
                      ${KDE4_KDEUI_LIBS} komain kritaimage ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES} ${QT_QTTEST_LIBRARY})
