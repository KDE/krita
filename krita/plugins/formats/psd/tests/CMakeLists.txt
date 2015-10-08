include_directories(${CMAKE_BINARY_DIR}/krita/libpsd)  #For kispsd_include.h

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_SOURCE_DIR}/..    ${CMAKE_SOURCE_DIR}/krita/sdk/tests )

macro_add_unittest_definitions()

if (WIN32)
    set(PSD_TEST_LIBS kritaglobal Qt5::Test ${WIN32_PLATFORM_NET_LIBS})
else (WIN32)
    set(PSD_TEST_LIBS kritaglobal Qt5::Test)
endif (WIN32)

########### next target ###############

set(psd_header_test_SRCS psd_header_test.cpp ${CMAKE_SOURCE_DIR}/krita/libpsd/psd_utils.cpp ../psd_header.cpp )
kde4_add_unit_test(psd_header_test TESTNAME krita-psd-psd_header_test  ${psd_header_test_SRCS})
target_link_libraries(psd_header_test ${PSD_TEST_LIBS})

########### next target ###############

set(psd_utils_test_SRCS psd_utils_test.cpp ${CMAKE_SOURCE_DIR}/krita/libpsd/psd_utils.cpp)
kde4_add_unit_test(psd_utils_test TESTNAME krita-psd-psd_utils_test  ${psd_utils_test_SRCS})
target_link_libraries(psd_utils_test ${PSD_TEST_LIBS})

########### next target ###############

set(psd_colormode_block_test_SRCS psd_colormode_block_test.cpp ../psd_colormode_block.cpp ../psd_header.cpp ${CMAKE_SOURCE_DIR}/krita/libpsd/psd_utils.cpp)
kde4_add_unit_test(psd_colormode_block_test TESTNAME krita-psd-psd_colormode_block_test  ${psd_colormode_block_test_SRCS})
target_link_libraries(psd_colormode_block_test ${PSD_TEST_LIBS})

########### next target ###############

set(compression_test_SRCS compression_test.cpp ${CMAKE_SOURCE_DIR}/krita/libpsd/compression.cpp)
kde4_add_unit_test(compression_test TESTNAME krita-psd-compression_test  ${compression_test_SRCS})
target_link_libraries(compression_test ${PSD_TEST_LIBS})

########### next target ###############
set(kis_psd_test_SRCS kis_psd_test.cpp )
kde4_add_unit_test(kis_psd_test TESTNAME krita-plugins-formats-psd_test ${kis_psd_test_SRCS})
target_link_libraries(kis_psd_test ${PSD_TEST_LIBS} kritaui)
