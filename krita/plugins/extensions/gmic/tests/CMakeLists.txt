set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(${KOMAIN_INCLUDES} ${CMAKE_SOURCE_DIR}/krita/sdk/tests ../)

macro_add_unittest_definitions()

set(kritagmic_parser_PART_SRCS
../kis_gmic_parser.cpp
../kis_gmic_blacklister.cpp
../Category.cpp
../Command.cpp
../Parameter.cpp
../kis_gmic_filter_model.cpp
../kis_gmic_filter_settings.cpp
../kis_gmic_simple_convertor.cpp
../kis_input_output_mapper.cpp
)


########### next target ###############

set(kis_gmic_tests_SRCS kis_gmic_tests.cpp ${kritagmic_parser_PART_SRCS})
kde4_add_unit_test(KisGmicTests TESTNAME krita-plugins-extensions-gmic-GmicTests ${kis_gmic_tests_SRCS})
target_link_libraries(KisGmicTests gmic kritaimage ${QT_QTTEST_LIBRARY} ${QT_QTXML_LIBRARY})


set(kis_gmic_benchmarks_SRCS kis_gmic_benchmarks.cpp ${kritagmic_parser_PART_SRCS})
kde4_add_unit_test(KisGmicBenchmark TESTNAME krita-plugins-extensions-gmic-GmicBenchmarks ${kis_gmic_benchmarks_SRCS})
target_link_libraries(KisGmicBenchmark gmic kritaimage ${QT_QTTEST_LIBRARY} ${QT_QTXML_LIBRARY})
