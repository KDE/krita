set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_SOURCE_DIR}/krita/sdk/tests ../)

macro_add_unittest_definitions()

set(kritagmic_parser_SOURCES
../kis_gmic_parser.cpp
../kis_gmic_blacklister.cpp
../Category.cpp
../Command.cpp
../Parameter.cpp
../kis_gmic_filter_model.cpp
../kis_gmic_filter_settings.cpp
../kis_gmic_simple_convertor.cpp
../kis_input_output_mapper.cpp
../kis_gmic_applicator.cpp
../kis_gmic_command.cpp
../kis_gmic_data.cpp
../kis_gmic_synchronize_layers_command.cpp
../kis_gmic_synchronize_image_size_command.cpp
../kis_export_gmic_processing_visitor.cpp
../kis_import_gmic_processing_visitor.cpp
)


########### next target ###############

set(kis_gmic_tests_SRCS kis_gmic_tests.cpp ${kritagmic_parser_SOURCES})
kde4_add_unit_test(KisGmicTests TESTNAME krita-plugins-extensions-gmic-GmicTests ${kis_gmic_tests_SRCS})
target_link_libraries(KisGmicTests gmic kritaimage Qt5::Test ${QT_QTXML_LIBRARY})


set(kis_gmic_benchmarks_SRCS kis_gmic_benchmarks.cpp ${kritagmic_parser_SOURCES})
calligra_add_benchmark(KisGmicBenchmark TESTNAME krita-plugins-extensions-gmic-GmicBenchmarks ${kis_gmic_benchmarks_SRCS})
target_link_libraries(KisGmicBenchmark gmic kritaimage Qt5::Test ${QT_QTXML_LIBRARY})
