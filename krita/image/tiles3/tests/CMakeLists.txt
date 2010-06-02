set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(  ${KOMAIN_INCLUDES} ${KOGUIUTILS_INCLUDES} )

########### next target ###############
set(kis_tiled_data_manager_test_SRCS kis_tiled_data_manager_test.cpp )
kde4_add_unit_test(KisTiledDataManagerTest TESTNAME krita-image-KisTiledDataManagerTest  ${kis_tiled_data_manager_test_SRCS})
target_link_libraries(KisTiledDataManagerTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})
