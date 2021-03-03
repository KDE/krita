########### next target ###############
include_directories(${CMAKE_SOURCE_DIR}/sdk/tests)

set(ko_colorspaces_benchmark_SRCS KoColorSpacesBenchmark.cpp)
krita_add_benchmark(KoColorSpacesBenchmark TESTNAME pigment-benchmarks-KoColorSpacesBenchmark ${ko_colorspaces_benchmark_SRCS})
target_link_libraries(KoColorSpacesBenchmark kritapigment KF5::I18n  Qt5::Test)

set(ko_compositeops_benchmark_SRCS KoCompositeOpsBenchmark.cpp)
krita_add_benchmark(KoCompositeOpsBenchmark TESTNAME pigment-benchmarks-KoCompositeOpsBenchmark ${ko_compositeops_benchmark_SRCS})
target_link_libraries(KoCompositeOpsBenchmark  kritapigment KF5::I18n  Qt5::Test)

