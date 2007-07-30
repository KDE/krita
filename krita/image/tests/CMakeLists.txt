include_directories(  ${KOMAIN_INCLUDES} ${KOGUIUTILS_INCLUDES} )

########### next target ###############

set(kis_layer_test_SRCS kis_layer_test.cpp )

kde4_add_unit_test(KisLayerTest TESTNAME krita-image-KisLayerTest  ${kis_layer_test_SRCS})

target_link_libraries(KisLayerTest  ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_image_tester_SRCS kis_image_tester.cpp )


kde4_add_unit_test(KisImageTester TESTNAME krita-image-KisImageTester  ${kis_image_tester_SRCS})

target_link_libraries(KisImageTester  ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_integer_maths_tester_SRCS kis_integer_maths_tester.cpp )


kde4_add_unit_test(KisIntegerMathsTester TESTNAME krita-image-KisIntegerMathsTester  ${kis_integer_maths_tester_SRCS})

target_link_libraries(KisIntegerMathsTester  ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_filter_configuration_tester_SRCS kis_filter_configuration_tester.cc )


kde4_add_unit_test(KisFilterConfigurationTester TESTNAME krita-image-KisFilterConfigurationTester  ${kis_filter_configuration_tester_SRCS})

target_link_libraries(KisFilterConfigurationTester  ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_effect_mask_test_SRCS kis_effect_mask_test.cpp )


kde4_add_unit_test(KisEffectMaskTest TESTNAME krita-image-KisEffectMaskTest  ${kis_effect_mask_test_SRCS})

target_link_libraries(KisEffectMaskTest  ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############
set(kis_iterator_test_SRCS kis_iterator_test.cpp )

kde4_add_unit_test(KisIteratorTest TESTNAME krita-image-KisIteratorTest  ${kis_iterator_test_SRCS})

target_link_libraries(KisIteratorTest  ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_image_view_converter_test_SRCS kis_image_view_converter_test.cpp )


kde4_add_unit_test(KisImage_View_ConverterTest TESTNAME krita-image-KisImage_View_ConverterTest  ${kis_image_view_converter_test_SRCS})

target_link_libraries(KisImage_View_ConverterTest  ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_painter_test_SRCS kis_painter_test.cpp )


kde4_add_unit_test(Kis_Painter_Test TESTNAME krita-image-Painter-Test ${kis_painter_test_SRCS})

target_link_libraries(Kis_Painter_Test  ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_threaded_applicator_test_SRCS kis_threaded_applicator_test.cpp )


kde4_add_unit_test(Kis_Threaded_Applicator_Test TESTNAME krita-image-Threaded_Applicator-Test ${kis_threaded_applicator_test_SRCS})

target_link_libraries(Kis_Threaded_Applicator_Test  ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_painterly_overlay_test_SRCS kis_painterly_overlay_test.cpp )


kde4_add_unit_test(Kis_Painterly_Overlay_Test TESTNAME krita-image-Painterly-Overlay-Test ${kis_painterly_overlay_test_SRCS})

target_link_libraries(Kis_Painterly_Overlay_Test  ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_selection_test_SRCS kis_selection_test.cpp )


kde4_add_unit_test(Kis_Selection_Test TESTNAME krita-image-Selection-Test ${kis_selection_test_SRCS})

target_link_libraries(Kis_Selection_Test  ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_node_test_SRCS kis_node_test.cpp )

kde4_add_unit_test(Kis_Node_Test TESTNAME krita-image-Node-Test ${kis_node_test_SRCS})

target_link_libraries(Kis_Node_Test  ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_node_facade_test_SRCS kis_node_facade_test.cpp )

kde4_add_unit_test(Kis_Node_Facade_Test TESTNAME krita-image-Node_Facade-Test ${kis_node_facade_test_SRCS})

target_link_libraries(Kis_Node_Facade_Test  ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############
