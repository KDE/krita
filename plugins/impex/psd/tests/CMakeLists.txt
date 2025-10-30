set(PSD_TEST_LIBS kritapsd kritapsd_static kritatestsdk)

kis_add_test(psd_header_test.cpp
    TEST_NAME psd_header_test
    LINK_LIBRARIES kritaglobal KF${KF_MAJOR}::I18n ${PSD_TEST_LIBS}
    )

kis_add_test(psd_colormode_block_test.cpp ../psd_colormode_block.cpp
    TEST_NAME psd_colormode_block_test
    LINK_LIBRARIES kritaglobal KF${KF_MAJOR}::I18n Qt${QT_MAJOR_VERSION}::Gui ${PSD_TEST_LIBS}
    )

kis_add_test(kis_psd_test.cpp
    TEST_NAME kis_psd_test
    LINK_LIBRARIES ${PSD_TEST_LIBS} kritaui
    )
