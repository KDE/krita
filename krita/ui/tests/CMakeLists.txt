
if(KDE4_BUILD_TESTS)

include_directories(  ${KOMAIN_INCLUDES} ${KOGUIUTILS_INCLUDES} )

########### next target ###############

set(kis_layer_model_test_SRCS kis_layer_model_test.cpp )

kde4_automoc(${kis_layer_model_test_SRCS})

kde4_add_executable(kis_layer_model_test RUN_UNINSTALLED ${kis_layer_model_test_SRCS})

target_link_libraries(kis_layer_model_test  ${KDE4_KDEUI_LIBS} koguiutils komain kritaui ${QT_QTTEST_LIBRARY})

add_test(krita-ui-kis_layer_model_test kis_layer_model_test)

########### next target ###############

set(kis_shape_controller_test_SRCS kis_shape_controller_test.cpp )

kde4_automoc(${kis_shape_controller_test_SRCS})

kde4_add_executable(kis_shape_controller_test RUN_UNINSTALLED ${kis_shape_controller_test_SRCS})

target_link_libraries(kis_shape_controller_test  ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage kritaui ${QT_QTTEST_LIBRARY})

add_test(krita-ui-kis_shape_controller_test kis_shape_controller_test)

########### end ###############

endif(KDE4_BUILD_TESTS)
