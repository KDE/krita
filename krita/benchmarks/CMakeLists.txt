set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(  ${KOMAIN_INCLUDES}  ${CMAKE_SOURCE_DIR}/krita/image/tiles3 )

########### next target ###############

set(kis_datamanager_benchmark_SRCS kis_datamanager_benchmark.cpp )
kde4_add_unit_test(KisDatamanagerBenchmark TESTNAME krita-benchmarks-KisDataManager ${kis_datamanager_benchmark_SRCS})
target_link_libraries(KisDatamanagerBenchmark ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

