if(KDE4_BUILD_TESTS)

include_directories(  ${KOMAIN_INCLUDES} ${KOGUIUTILS_INCLUDES} )

########### next target ###############

set(kis_layer_test_SRCS kis_layer_test.cpp )

kde4_automoc(${kis_layer_test_SRCS})

kde4_add_executable(KisLayerTest RUN_UNINSTALLED ${kis_layer_test_SRCS})

target_link_libraries(KisLayerTest  ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage ${QT_QTTEST_LIBRARY})

add_test(krita-image-KisLayerTest KisLayerTest)

########### next target ###############

set(kis_image_tester_SRCS kis_image_tester.cpp )

kde4_automoc(${kis_image_tester_SRCS})

kde4_add_executable(KisImageTester RUN_UNINSTALLED ${kis_image_tester_SRCS})

target_link_libraries(KisImageTester  ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage ${QT_QTTEST_LIBRARY})

add_test(krita-image-KisImageTester KisImageTester)

########### next target ###############

set(kis_integer_maths_tester_SRCS kis_integer_maths_tester.cpp )

kde4_automoc(${kis_integer_maths_tester_SRCS})

kde4_add_executable(KisIntegerMathsTester RUN_UNINSTALLED ${kis_integer_maths_tester_SRCS})

target_link_libraries(KisIntegerMathsTester  ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage ${QT_QTTEST_LIBRARY})

add_test(krita-image-KisIntegerMathsTester KisIntegerMathsTester)

########### next target ###############

set(kis_filter_configuration_tester_SRCS kis_filter_configuration_tester.cc )

kde4_automoc(${kis_filter_configuration_tester_SRCS})

kde4_add_executable(KisFilterConfigurationTester RUN_UNINSTALLED ${kis_filter_configuration_tester_SRCS})

target_link_libraries(KisFilterConfigurationTester  ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage ${QT_QTTEST_LIBRARY})

add_test(krita-image-KisFilterConfigurationTester KisFilterConfigurationTester)

########### next target ###############

set(kis_effect_mask_test_SRCS kis_effect_mask_test.cpp )

kde4_automoc(${kis_effect_mask_test_SRCS})

kde4_add_executable(KisEffectMaskTest RUN_UNINSTALLED ${kis_effect_mask_test_SRCS})

target_link_libraries(KisEffectMaskTest  ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage ${QT_QTTEST_LIBRARY})

add_test(krita-image-KisEffectMaskTest KisEffectMaskTest)

########### next target ###############


endif(KDE4_BUILD_TESTS)
