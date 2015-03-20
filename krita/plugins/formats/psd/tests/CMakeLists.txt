set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_SOURCE_DIR}/..    ${CMAKE_SOURCE_DIR}/krita/sdk/tests )

macro_add_unittest_definitions()

########### next target ###############

set(psd_header_test_SRCS psd_header_test.cpp ../psd_utils.cpp ../psd_header.cpp )
add_executable(psd_header_testNAME krita-psd-psd_header_test  ${psd_header_test_SRCS})
add_test(psd_header_test psd_header_test)
ecm_mark_as_test(psd_header_test)
target_link_libraries(psd_header_test  Qt5::Test)

########### next target ###############

set(psd_utils_test_SRCS psd_utils_test.cpp ../psd_utils.cpp )
add_executable(psd_utils_testNAME krita-psd-psd_utils_test  ${psd_utils_test_SRCS})
add_test(psd_utils_test psd_utils_test)
ecm_mark_as_test(psd_utils_test)
target_link_libraries(psd_utils_test  Qt5::Test)

########### next target ###############

set(psd_colormode_block_test_SRCS psd_colormode_block_test.cpp ../psd_colormode_block.cpp ../psd_header.cpp ../psd_utils.cpp)
add_executable(psd_colormode_block_testNAME krita-psd-psd_colormode_block_test  ${psd_colormode_block_test_SRCS})
add_test(psd_colormode_block_test psd_colormode_block_test)
ecm_mark_as_test(psd_colormode_block_test)
target_link_libraries(psd_colormode_block_test  Qt5::Test)

########### next target ###############

set(compression_test_SRCS compression_test.cpp ../compression.cpp)
add_executable(compression_testNAME krita-psd-compression_test  ${compression_test_SRCS})
add_test(compression_test compression_test)
ecm_mark_as_test(compression_test)
target_link_libraries(compression_test  Qt5::Test)

########### next target ###############
#set(kis_psd_test_SRCS kis_psd_test.cpp )
#add_executable(kis_psd_testNAME krita-plugins-formats-psd_test ${kis_psd_test_SRCS})
add_test(kis_psd_test kis_psd_test)
ecm_mark_as_test(kis_psd_test)
#target_link_libraries(kis_psd_test   kritaui Qt5::Test)
