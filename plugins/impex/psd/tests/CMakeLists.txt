include_directories(${CMAKE_BINARY_DIR}/libs/psd)  #For kispsd_include.h
include_directories(${CMAKE_BINARY_DIR}/libs/pigment)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(
    ${CMAKE_SOURCE_DIR}/..
    ${CMAKE_SOURCE_DIR}/sdk/tests
    ${CMAKE_SOURCE_DIR}/libs/psd
    ${CMAKE_SOURCE_DIR}/plugins/impex/psd
    ${CMAKE_SOURCE_DIR}/libs/pigment
)

macro_add_unittest_definitions()

if (WIN32)
    set(PSD_TEST_LIBS kritapsd Qt5::Test ${WIN32_PLATFORM_NET_LIBS})
else (WIN32)
    set(PSD_TEST_LIBS kritapsd Qt5::Test)
endif (WIN32)

ecm_add_tests(
    psd_utils_test.cpp
    compression_test.cpp
    NAME_PREFIX "plugins-impex-psd-"
    LINK_LIBRARIES ${PSD_TEST_LIBS})

ecm_add_test(psd_header_test.cpp ../psd_header.cpp
    TEST_NAME plugins-impex-psd-psd_header_test
    LINK_LIBRARIES kritaglobal KF5::I18n ${PSD_TEST_LIBS})

ecm_add_test(psd_colormode_block_test.cpp ../psd_header.cpp ../psd_colormode_block.cpp
    TEST_NAME plugins-impex-psd-psd_colormode_block_test
    LINK_LIBRARIES kritaglobal KF5::I18n Qt5::Gui ${PSD_TEST_LIBS})

krita_add_broken_unit_test(kis_psd_test.cpp
    TEST_NAME plugins-impex-formats-psd_test
    LINK_LIBRARIES ${PSD_TEST_LIBS} kritaui)
