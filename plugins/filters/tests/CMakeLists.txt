set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(    )

macro_add_unittest_definitions()

########### next target ###############

set(kis_all_filter_test_SRCS kis_all_filter_test.cpp )
kde4_add_executable(KisAllFilterTest TEST ${kis_all_filter_test_SRCS})
target_link_libraries(KisAllFilterTest  kritaimage Qt5::Test)

########### next target ###############

set(kis_crash_filter_test_SRCS kis_crash_filter_test.cpp )
kde4_add_executable(KisCrashFilterTest TEST ${kis_crash_filter_test_SRCS})
target_link_libraries(KisCrashFilterTest  kritaimage Qt5::Test)
