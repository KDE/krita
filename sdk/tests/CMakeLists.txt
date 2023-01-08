add_library(kritatestsdk INTERFACE)

target_link_libraries(kritatestsdk INTERFACE Qt5::Test)

include(KritaAddBrokenUnitTest)
include(MacroKritaAddBenchmark)

set(KRITA_DEFAULT_TEST_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/data/)

target_compile_definitions(kritatestsdk INTERFACE 
    FILES_DEFAULT_DATA_DIR="${KRITA_DEFAULT_TEST_DATA_DIR}"
    SYSTEM_RESOURCES_DATA_DIR="${CMAKE_SOURCE_DIR}/krita/data/"
)

macro(macro_add_unittest_definitions)
    add_compile_definitions(FILES_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/")
    if(WIN32)
        add_compile_definitions(FILES_OUTPUT_DIR="${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    else()
        add_compile_definitions(FILES_OUTPUT_DIR="${CMAKE_CURRENT_BINARY_DIR}")
    endif()
endmacro(macro_add_unittest_definitions)
