set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

########### next target ###############

set(ko_colorspaces_benchmark_SRCS KoColorSpacesBenchmark.cpp)
kde4_add_unit_test(KoColorSpacesBenchmark TESTNAME pigment-benchmarks-KoColorSpacesBenchmark ${ko_colorspaces_benchmark_SRCS})
target_link_libraries(KoColorSpacesBenchmark pigmentcms ${QT_QTTEST_LIBRARY})


