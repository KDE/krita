
if(KDE4_BUILD_TESTS)

include_directories(  ${KOFFICECORE_INCLUDES} ${KOFFICEUI_INCLUDES} )

########### next target ###############

set(kis_layer_model_test_SRCS kis_layer_model_test.cpp )

kde4_automoc(${kis_layer_model_test_SRCS})

kde4_add_executable(kis_layer_model_test RUN_UNINSTALLED ${kis_layer_model_test_SRCS})

target_link_libraries(kis_layer_model_test  ${KDE4_KDEUI_LIBS} kofficeui kofficecore kritaui ${QT_QTTEST_LIBRARY})

add_test(libs-kofficeui-kis_layer_model_test kis_layer_model_test)

########### end ###############

endif(KDE4_BUILD_TESTS)
