include_directories(
    ${CMAKE_SOURCE_DIR}/plugins/impex/psd
)

macro_add_unittest_definitions()

set(PSD_TEST_LIBS kritapsd kritatestsdk)

kis_add_test(psd_header_test.cpp
    TEST_NAME psd_header_test
    LINK_LIBRARIES kritaglobal KF5::I18n ${PSD_TEST_LIBS}
    NAME_PREFIX "plugins-impex-psd-")

kis_add_test(psd_colormode_block_test.cpp ../psd_colormode_block.cpp
    TEST_NAME psd_colormode_block_test
    LINK_LIBRARIES kritaglobal KF5::I18n Qt5::Gui ${PSD_TEST_LIBS}
    NAME_PREFIX "plugins-impex-psd-")

kis_add_test(kis_psd_test.cpp
    TEST_NAME kis_psd_test
    LINK_LIBRARIES ${PSD_TEST_LIBS} kritaui
    NAME_PREFIX "plugins-impex-psd-")
