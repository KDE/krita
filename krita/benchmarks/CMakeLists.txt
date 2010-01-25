set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(  ${KOMAIN_INCLUDES}  ${CMAKE_SOURCE_DIR}/krita/image/tiles3 )

add_definitions(-DFILES_DATA_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/data/\\"")
add_definitions(-DFILES_OUTPUT_DIR="\\"${CMAKE_CURRENT_BINARY_DIR}\\"")


########### next target ###############

set(kis_datamanager_benchmark_SRCS kis_datamanager_benchmark.cpp)
set(kis_iterator_benchmark_SRCS kis_hline_iterator_benchmark.cpp)
set(kis_projection_benchmark_SRCS kis_projection_benchmark.cpp)


kde4_add_unit_test(KisDatamanagerBenchmark TESTNAME krita-benchmarks-KisDataManager ${kis_datamanager_benchmark_SRCS})
kde4_add_unit_test(KisHLineIteratorBenchmark TESTNAME krita-benchmarks-KisHLineIterator ${kis_iterator_benchmark_SRCS})
kde4_add_unit_test(KisProjectionBenchmark TESTNAME krita-benchmarks-KisProjectionBenchmark ${kis_projection_benchmark_SRCS})

target_link_libraries(KisDatamanagerBenchmark ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})
target_link_libraries(KisHLineIteratorBenchmark ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})
target_link_libraries(KisProjectionBenchmark ${KDE4_KDEUI_LIBS} kritaimage kritaui ${QT_QTTEST_LIBRARY})