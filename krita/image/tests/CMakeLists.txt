set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(  ${KOMAIN_INCLUDES} ${KOGUIUTILS_INCLUDES} )
########### next target ###############

set(kis_sharedptr_test_SRCS kis_sharedptr_test.cpp )

kde4_add_unit_test(KisSharedPtrTest TESTNAME krita-image-KisSharedPtrTest ${kis_sharedptr_test_SRCS})

target_link_libraries(KisSharedPtrTest  ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_base_node_test_SRCS kis_base_node_test.cpp )

kde4_add_unit_test(KisBaseNodeTest TESTNAME krita-image-BaseNodeTest ${kis_base_node_test_SRCS})

target_link_libraries(KisBaseNodeTest  ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_node_test_SRCS kis_node_test.cpp )

kde4_add_unit_test(KisNodeTest TESTNAME krita-image-Node-Test ${kis_node_test_SRCS})

target_link_libraries(KisNodeTest  ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_node_facade_test_SRCS kis_node_facade_test.cpp )

kde4_add_unit_test(KisNodeFacadeTest TESTNAME krita-image-NodeFacadeTest ${kis_node_facade_test_SRCS})

target_link_libraries(KisNodeFacadeTest  ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

add_definitions(-DFILES_DATA_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/data/\\"")

set(kis_paint_device_test_SRCS kis_paint_device_test.cpp )

kde4_add_unit_test(KisPaintDeviceTest TESTNAME krita-image-KisPaintDeviceTest ${kis_paint_device_test_SRCS})

target_link_libraries(KisPaintDeviceTest  ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage ${QT_QTTEST_LIBRARY})

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

set(kis_painter_test_SRCS kis_painter_test.cpp )


kde4_add_unit_test(KisPainterTest TESTNAME krita-image-KisPainterTest ${kis_painter_test_SRCS})

target_link_libraries(KisPainterTest  ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_threaded_applicator_test_SRCS kis_threaded_applicator_test.cpp )

kde4_add_unit_test(KisThreadedApplicatorTest TESTNAME krita-image-KisThreadedApplicatorTest ${kis_threaded_applicator_test_SRCS})

target_link_libraries(KisThreadedApplicatorTest  ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_painterly_overlay_test_SRCS kis_painterly_overlay_test.cpp )

kde4_add_unit_test(KisPainterlyOverlayTest TESTNAME krita-image-PainterlyOverlayTest ${kis_painterly_overlay_test_SRCS})

target_link_libraries(KisPainterlyOverlayTest  ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

#set(kis_selection_test_SRCS kis_selection_test.cpp )

#kde4_add_unit_test(KisSelectionTest TESTNAME krita-image-SelectionTest ${kis_selection_test_SRCS})

#target_link_libraries(KisSelectionTest  ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_count_visitor_test_SRCS kis_count_visitor_test.cpp )

kde4_add_unit_test(KisCountVisitorTest TESTNAME krita-image-KisCountVisitorTest ${kis_count_visitor_test_SRCS})

target_link_libraries(KisCountVisitorTest  ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############
set(kis_projection_test_SRCS kis_projection_test.cpp )

kde4_add_unit_test(KisProjectionTest TESTNAME krita-image-KisProjectionTest ${kis_projection_test_SRCS})

target_link_libraries(KisProjectionTest  ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_properties_configuration_test_SRCS kis_properties_configuration_test.cpp )

kde4_add_unit_test(Kis_Properties_Configuration_Test TESTNAME krita-image-properties-configuration-Test ${kis_properties_configuration_test_SRCS})

target_link_libraries(Kis_Properties_Configuration_Test  ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_transaction_test_SRCS kis_transaction_test.cpp )

kde4_add_unit_test(KisTransactionTest TESTNAME krita-image-KisTransactionTest ${kis_transaction_test_SRCS})

target_link_libraries(KisTransactionTest  ${KDE4_KDEUI_LIBS} komain kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_pixel_selection_test_SRCS kis_pixel_selection_test.cpp )

kde4_add_unit_test(KisPixelSelectionTest TESTNAME krita-image-KisPixelSelectionTest ${kis_pixel_selection_test_SRCS})

target_link_libraries(KisPixelSelectionTest  ${KDE4_KDEUI_LIBS} komain kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_group_layer_test_SRCS kis_group_layer_test.cpp )

kde4_add_unit_test(KisGroupLayerTest TESTNAME krita-image-KisGroupLayerTest ${kis_group_layer_test_SRCS})

target_link_libraries(KisGroupLayerTest  ${KDE4_KDEUI_LIBS} komain kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_filter_mask_test_SRCS kis_filter_mask_test.cpp )

kde4_add_unit_test(KisFilterMaskTest TESTNAME krita-image-KisFilterMaskTest ${kis_filter_mask_test_SRCS})

target_link_libraries(KisFilterMaskTest  ${KDE4_KDEUI_LIBS} komain kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_paint_layer_test_SRCS kis_paint_layer_test.cpp )

kde4_add_unit_test(KisPaintLayerTest TESTNAME krita-image-KisPaintLayerTest ${kis_paint_layer_test_SRCS})

target_link_libraries(KisPaintLayerTest  ${KDE4_KDEUI_LIBS} komain kritaimage ${QT_QTTEST_LIBRARY})
