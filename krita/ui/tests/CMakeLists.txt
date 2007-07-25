include_directories(  ${KOMAIN_INCLUDES} ${KOGUIUTILS_INCLUDES} )

########### next target ###############

set(kis_layer_model_test_SRCS kis_layer_model_test.cpp )

kde4_add_unit_test(kis_layer_model_test TESTNAME krita-ui-kis_layer_model_test  ${kis_layer_model_test_SRCS})

target_link_libraries(kis_layer_model_test  ${KDE4_KDEUI_LIBS} koguiutils komain kritaui ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_shape_controller_test_SRCS kis_shape_controller_test.cpp )


kde4_add_unit_test(kis_shape_controller_test TESTNAME krita-ui-kis_shape_controller_test  ${kis_shape_controller_test_SRCS})

target_link_libraries(kis_shape_controller_test  ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage kritaui ${QT_QTTEST_LIBRARY})

########### next target ###############

set(squeezedcombobox_test_SRCS squeezedcombobox_test.cpp )

kde4_add_unit_test(squeezedcombobox_test TESTNAME krita-ui-squeezedcombobox_test  ${squeezedcombobox_test_SRCS})

target_link_libraries(squeezedcombobox_test  ${KDE4_KDEUI_LIBS} kritaui ${QT_QTTEST_LIBRARY})

########### end ###############
