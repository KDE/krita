set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(  ${KOMAIN_INCLUDES}   ${CMAKE_SOURCE_DIR}/krita/sdk/tests )

add_definitions(-DFILES_DATA_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/data/\\"")

########### next target ###############

set(kis_node_model_test_SRCS kis_node_model_test.cpp modeltest.cpp )

kde4_add_unit_test(kis_node_model_test TESTNAME krita-ui-kis_node_model_test  ${kis_node_model_test_SRCS})

target_link_libraries(kis_node_model_test  ${KDE4_KDEUI_LIBS}  komain kritaui ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_image_view_converter_test_SRCS kis_image_view_converter_test.cpp )

kde4_add_unit_test(KisImageViewConverterTest TESTNAME krita-ui-KisImageViewConverterTest  ${kis_image_view_converter_test_SRCS})

target_link_libraries(KisImageViewConverterTest  ${KDE4_KDEUI_LIBS}  komain kritaui ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_shape_controller_test_SRCS kis_shape_controller_test.cpp )

kde4_add_unit_test(kis_shape_controller_test TESTNAME krita-ui-kis_shape_controller_test  ${kis_shape_controller_test_SRCS})

target_link_libraries(kis_shape_controller_test  ${KDE4_KDEUI_LIBS}  komain kritaimage kritaui ${QT_QTTEST_LIBRARY})

########### next target ###############

set(squeezedcombobox_test_SRCS squeezedcombobox_test.cpp )

kde4_add_unit_test(squeezedcombobox_test TESTNAME krita-ui-squeezedcombobox_test  ${squeezedcombobox_test_SRCS})

target_link_libraries(squeezedcombobox_test  ${KDE4_KDEUI_LIBS} kritaui ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_prescaled_projection_test_SRCS kis_prescaled_projection_test.cpp )

kde4_add_unit_test(KisPrescaledProjectionTest TESTNAME krita-ui-kis_prescaled_projection_test ${kis_prescaled_projection_test_SRCS})

target_link_libraries(KisPrescaledProjectionTest  ${KDE4_KDEUI_LIBS} kritaui ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_kra_loader_test_SRCS kis_kra_loader_test.cpp )
kde4_add_unit_test(KisKraLoaderTest TESTNAME krita-ui-KisKraLoaderTest ${kis_kra_loader_test_SRCS})
target_link_libraries(KisKraLoaderTest  ${KDE4_KDEUI_LIBS} komain kritaimage kritaui ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_kra_saver_test_SRCS kis_kra_saver_test.cpp )
kde4_add_unit_test(KisKraSaverTest TESTNAME krita-ui-KisKraSaverTest ${kis_kra_saver_test_SRCS})
target_link_libraries(KisKraSaverTest  ${KDE4_KDEUI_LIBS} komain kritaimage kritaui ${QT_QTTEST_LIBRARY})


########### next target ###############

set(kis_kra_savexml_visitor_test_SRCS kis_kra_savexml_visitor_test.cpp )
kde4_add_unit_test(KisKraSaveXmlVisitorTest TESTNAME krita-ui-KisKraSaveXmlVisitorTest ${kis_kra_savexml_visitor_test_SRCS})
target_link_libraries(KisKraSaveXmlVisitorTest  ${KDE4_KDEUI_LIBS} komain kritaimage kritaui ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_shape_selection_test_SRCS kis_shape_selection_test.cpp )

kde4_add_unit_test(KisShapeSelectionTest TESTNAME krita-ui-ShapeSelectionTest ${kis_shape_selection_test_SRCS})

target_link_libraries(KisShapeSelectionTest  ${KDE4_KDEUI_LIBS} kritaimage kritaui ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_autogradient_resource_test_SRCS kis_autogradient_resource_test.cpp )
kde4_add_unit_test(KisAutogradientResourceTest TESTNAME krita-ui-KisAutogradientResourceTest ${kis_autogradient_resource_test_SRCS})
target_link_libraries(KisAutogradientResourceTest  ${KDE4_KDEUI_LIBS} kritaimage kritaui ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_exiv2_test_SRCS kis_exiv2_test.cpp )
kde4_add_unit_test(KisExiv2Test TESTNAME krita-ui-KisExiv2Test ${kis_exiv2_test_SRCS})
target_link_libraries(KisExiv2Test  ${KDE4_KDEUI_LIBS} kritaimage kritaui ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_recorded_action_editor_test_SRCS kis_recorded_action_editor_test.cpp )
kde4_add_unit_test(KisRecordedActionEditorTest TESTNAME krita-ui-KisRecordedActionEditor ${kis_recorded_action_editor_test_SRCS})
target_link_libraries(KisRecordedActionEditorTest  ${KDE4_KDEUI_LIBS} kritaimage kritaui ${QT_QTTEST_LIBRARY})
